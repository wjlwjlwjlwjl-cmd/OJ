#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

BUILD_JUDGE="${BUILD_JUDGE:-1}"
BUILD_BACKEND="${BUILD_BACKEND:-1}"
JUDGE_PORT="${JUDGE_PORT:-9090}"
BACKEND_PORT="${BACKEND_PORT:-8080}"

cleanup() {
    echo ""
    echo "Shutting down OJ services..."
    if [ -n "$JUDGE_PID" ] && kill -0 "$JUDGE_PID" 2>/dev/null; then
        echo "Stopping judge service (PID $JUDGE_PID)..."
        kill "$JUDGE_PID" 2>/dev/null
        wait "$JUDGE_PID" 2>/dev/null
        echo "Judge service stopped."
    fi
    if [ -n "$BACKEND_PID" ] && kill -0 "$BACKEND_PID" 2>/dev/null; then
        echo "Stopping backend (PID $BACKEND_PID)..."
        kill "$BACKEND_PID" 2>/dev/null
        wait "$BACKEND_PID" 2>/dev/null
        echo "Backend stopped."
    fi
    echo "All services stopped."
}

trap cleanup SIGINT SIGTERM EXIT

echo "=== OJ Platform ==="

# ── Build judge service ──
if [ "$BUILD_JUDGE" -eq 1 ]; then
    echo "[1/4] Building judge service..."
    cd "$PROJECT_DIR/judge"
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release 2>&1 | tail -5
    make -j$(nproc) 2>&1 | tail -5
    echo "       Judge build complete."
fi

# ── Build backend ──
if [ "$BUILD_BACKEND" -eq 1 ]; then
    echo "[2/4] Building backend server..."
    cd "$PROJECT_DIR/backend"
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release 2>&1 | tail -5
    make -j$(nproc) 2>&1 | tail -5
    echo "       Backend build complete."
fi

# ── Start judge service ──
echo "[3/4] Starting judge service on 127.0.0.1:$JUDGE_PORT..."
cd "$PROJECT_DIR/judge/build"
export CONFIG_PATH="$PROJECT_DIR/config/config.json"
nohup ./oj-judge > "$PROJECT_DIR/judge/judge.log" 2>&1 &
JUDGE_PID=$!

# Wait for judge to be ready
for i in $(seq 1 30); do
    if curl -s "http://127.0.0.1:$JUDGE_PORT/judge" -X POST \
        -H "Content-Type: application/json" \
        -d '{"code":"int main(){return 0;}","time_limit_ms":100,"test_cases":[]}' \
        >/dev/null 2>&1; then
        break
    fi
    sleep 0.5
done
echo "       Judge service started (PID $JUDGE_PID)."
echo "$JUDGE_PID" > "$PROJECT_DIR/judge/judge.pid"

# ── Start backend ──
echo "[4/4] Starting backend server on 0.0.0.0:$BACKEND_PORT..."
cd "$PROJECT_DIR/backend/build"
export CONFIG_PATH="$PROJECT_DIR/config/config.json"
nohup ./oj-backend > "$PROJECT_DIR/backend/backend.log" 2>&1 &
BACKEND_PID=$!
echo "       Backend started (PID $BACKEND_PID)."
echo "$BACKEND_PID" > "$PROJECT_DIR/backend/backend.pid"

echo ""
echo "=== OJ is running ==="
echo "  Frontend : http://localhost:$BACKEND_PORT"
echo "  Judge    : http://127.0.0.1:$JUDGE_PORT"
echo ""
echo "  Logs:"
echo "    Judge   : tail -f judge/judge.log"
echo "    Backend : tail -f backend/backend.log"
echo ""
echo "  Stop: ./scripts/stop.sh  (or Ctrl+C here)"
echo ""

wait
