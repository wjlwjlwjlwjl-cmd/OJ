#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "Starting OJ Backend..."

cd "$PROJECT_DIR/backend"

if [ ! -d "build" ]; then
    echo "Building backend..."
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    cd ..
fi

exec ./build/oj-backend
