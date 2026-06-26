#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

EXPORTED_FUNCTIONS='["_engine_init","_engine_process","_engine_set_step","_engine_set_base","_engine_set_lane","_engine_set_bpm","_engine_set_reverb","_engine_play","_engine_stop","_engine_get_step","_engine_set_macro","_malloc","_free"]'

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
