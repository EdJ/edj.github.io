#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

PORT=${1:-8080}

# Prefer emsdk over anything else in PATH (Homebrew emscripten is unreliable)
for _env in "$HOME/emsdk/emsdk_env.sh" "$HOME/.emsdk/emsdk_env.sh"; do
    if [ -f "$_env" ]; then
        # shellcheck source=/dev/null
        source "$_env" 2>/dev/null
        break
    fi
done
EMCC=$(command -v emcc 2>/dev/null || true)

# Build WASM if emcc is available and sources are newer than the output
if [ -n "$EMCC" ]; then
    if [ ! -f kick_engine.wasm ] || \
       [ rawaudiofiles/web/kick_engine.cpp -nt kick_engine.wasm ] || \
       [ rawaudiofiles/web/dsp/kick_web.hpp -nt kick_engine.wasm ]; then
        echo "Building WASM…"
        (cd rawaudiofiles/web && EMCC="$EMCC" ./build.sh)
    else
        echo "WASM up to date."
    fi
else
    echo "emcc not found — skipping WASM build (using existing kick_engine.wasm if present)"
    echo "  Run ./install-emsdk.sh to install the official toolchain"
fi

# Rebuild the bundle if it's missing or older than either source file
# (no emcc needed — just cat)
if [ ! -f kick-worklet-bundle.js ] || \
   [ kick_engine.js   -nt kick-worklet-bundle.js ] || \
   [ kick-worklet.js  -nt kick-worklet-bundle.js ]; then
    if [ -f kick_engine.js ] && [ -f kick-worklet.js ]; then
        echo "Bundling → kick-worklet-bundle.js"
        {
            printf 'var __kickBase=(function(){try{return new URL(".",self.location.href).href}catch(e){return "/"}})();\n'
            cat kick_engine.js
            printf '\n'
            cat kick-worklet.js
        } > kick-worklet-bundle.js
    else
        echo "Warning: kick_engine.js not found — bundle not generated"
        echo "  Run ./install-emsdk.sh then ./run.sh to build"
    fi
fi

# Pick whichever server is available
if command -v npx &>/dev/null; then
    echo "Serving on http://localhost:$PORT"
    npx --yes serve -l "$PORT" .
elif command -v python3 &>/dev/null; then
    echo "Serving on http://localhost:$PORT"
    python3 -m http.server "$PORT"
elif command -v python &>/dev/null; then
    echo "Serving on http://localhost:$PORT"
    python -m SimpleHTTPServer "$PORT"
else
    echo "No server found — install Node.js or Python" >&2
    exit 1
fi
