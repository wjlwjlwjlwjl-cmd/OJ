#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "Stopping OJ services..."

# Stop backend
if [ -f "$PROJECT_DIR/backend/backend.pid" ]; then
    BACKEND_PID=$(cat "$PROJECT_DIR/backend/backend.pid")
    if kill -0 "$BACKEND_PID" 2>/dev/null; then
        echo "Stopping backend (PID $BACKEND_PID)..."
        kill "$BACKEND_PID" 2>/dev/null
        echo "Backend stopped."
    fi
    rm -f "$PROJECT_DIR/backend/backend.pid"
fi

# Stop judge
if [ -f "$PROJECT_DIR/judge/judge.pid" ]; then
    JUDGE_PID=$(cat "$PROJECT_DIR/judge/judge.pid")
    if kill -0 "$JUDGE_PID" 2>/dev/null; then
        echo "Stopping judge service (PID $JUDGE_PID)..."
        kill "$JUDGE_PID" 2>/dev/null
        echo "Judge service stopped."
    fi
    rm -f "$PROJECT_DIR/judge/judge.pid"
fi

# Fallback: kill by process name
if pgrep -f "oj-judge" >/dev/null 2>&1; then
    echo "Stopping remaining judge processes..."
    pkill -f "oj-judge" 2>/dev/null || true
fi
if pgrep -f "oj-backend" >/dev/null 2>&1; then
    echo "Stopping remaining backend processes..."
    pkill -f "oj-backend" 2>/dev/null || true
fi

echo "All services stopped."
