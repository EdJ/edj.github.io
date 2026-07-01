#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

# Emscripten 6.x requires Python 3.8+; put python3.10 first in PATH if system python3 is older
_PYBIN=$(mktemp -d)
ln -sf /usr/local/bin/python3.10 "$_PYBIN/python3"
export PATH="$_PYBIN:$PATH"

EXPORTED_FUNCTIONS='["_engine_init","_engine_process","_engine_set_step","_engine_set_base","_engine_set_lane","_engine_set_bpm","_engine_set_reverb","_engine_play","_engine_stop","_engine_get_step","_engine_set_macro","_engine_set_swing","_engine_set_fm_step","_engine_set_fm_param","_engine_set_swarm_step","_engine_set_swarm_param","_engine_sampler_in_l","_engine_sampler_in_r","_engine_sampler_push","_malloc","_free"]'

${EMCC:-emcc} kick_engine.cpp \
    -I. \
    -std=c++17 \
    -O3 \
    -msimd128 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='KickEngineModule' \
    -s ENVIRONMENT='web,worker' \
    -s EXPORTED_FUNCTIONS="$EXPORTED_FUNCTIONS" \
    -s EXPORTED_RUNTIME_METHODS='["HEAPF32","HEAPU8"]' \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s INITIAL_MEMORY=16777216 \
    -o ../../kick_engine.js

echo "Compiled → kick_engine.js + kick_engine.wasm"

{
    printf 'var __kickBase=(function(){try{return new URL(".",self.location.href).href}catch(e){return "/"}})();\n'
    cat ../../kick_engine.js
    printf '\n'
    cat ../../kick-worklet.js
} > ../../kick-worklet-bundle.js

echo "Bundled  → kick-worklet-bundle.js"
