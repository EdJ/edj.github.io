#!/usr/bin/env bash
set -euo pipefail

EMSDK_DIR="$HOME/emsdk"

if [ -f "$EMSDK_DIR/emsdk_env.sh" ]; then
    echo "emsdk already installed at $EMSDK_DIR"
    source "$EMSDK_DIR/emsdk_env.sh"
    echo "emcc version: $(emcc --version | head -1)"
    exit 0
fi

echo "Cloning emsdk…"
git clone https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR"

cd "$EMSDK_DIR"
echo "Installing latest…"
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

echo ""
echo "Done. emcc version: $(emcc --version | head -1)"
echo ""
echo "To activate in your current shell, run:"
echo "  source ~/emsdk/emsdk_env.sh"
echo ""
echo "Or just use ./run.sh — it sources emsdk automatically."
