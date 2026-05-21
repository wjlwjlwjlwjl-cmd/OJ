#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "=== Building OJ ==="

echo "[1/3] Building judge service..."
cd "$PROJECT_DIR/judge"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
echo "       Done."

echo "[2/3] Building backend server..."
cd "$PROJECT_DIR/backend"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
echo "       Done."

echo "[3/3] Checking test builds..."
for test_dir in "$PROJECT_DIR/test"/todo-*/; do
    name=$(basename "$test_dir")
    if [ -f "$test_dir/CMakeLists.txt" ]; then
        mkdir -p "$test_dir/build"
        cd "$test_dir/build"
        cmake .. 2>&1 | tail -1
        make -j$(nproc) 2>&1 | tail -1
        echo "       $name: OK"
    fi
done

echo ""
echo "=== Build complete ==="
echo "  Judge    : judge/build/oj-judge"
echo "  Backend  : backend/build/oj-backend"
echo "  Run      : ./scripts/start.sh"
