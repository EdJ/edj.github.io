var __kickBase=(function(){try{return new URL(".",self.location.href).href}catch(e){return "/"}})();
var KickEngineModule=(()=>{var _scriptName=globalThis.document?.currentScript?.src;return async function(moduleArg={}){var Module=moduleArg;var ENVIRONMENT_IS_WEB=!!globalThis.window;var ENVIRONMENT_IS_WORKER=!!globalThis.WorkerGlobalScope;var ENVIRONMENT_IS_NODE=globalThis.process?.versions?.node&&globalThis.process?.type!="renderer";var programArgs=[];var thisProgram="./this.program";if(ENVIRONMENT_IS_WORKER){_scriptName=self.location.href}var scriptDirectory="";function locateFile(path){if(Module["locateFile"]){return Module["locateFile"](path,scriptDirectory)}return scriptDirectory+path}var readAsync,readBinary;if(ENVIRONMENT_IS_WEB||ENVIRONMENT_IS_WORKER){try{scriptDirectory=new URL(".",_scriptName).href}catch{}{if(ENVIRONMENT_IS_WORKER){readBinary=url=>{var xhr=new XMLHttpRequest;xhr.open("GET",url,false);xhr.responseType="arraybuffer";xhr.send(null);return new Uint8Array(xhr.response)}}readAsync=async url=>{var response=await fetch(url,{credentials:"same-origin"});if(response.ok){return response.arrayBuffer()}throw new Error(response.status+" : "+response.url)}}}else{}var out=console.log.bind(console);var err=console.error.bind(console);var wasmBinary;var ABORT=false;class EmscriptenEH{}class EmscriptenSjLj extends EmscriptenEH{}var runtimeInitialized=false;function updateMemoryViews(){var b=wasmMemory.buffer;HEAP8=new Int8Array(b);HEAP16=new Int16Array(b);Module["HEAPU8"]=HEAPU8=new Uint8Array(b);HEAPU16=new Uint16Array(b);HEAP32=new Int32Array(b);HEAPU32=new Uint32Array(b);Module["HEAPF32"]=HEAPF32=new Float32Array(b);HEAPF64=new Float64Array(b);HEAP64=new BigInt64Array(b);HEAPU64=new BigUint64Array(b)}function preRun(){if(Module["preRun"]){if(typeof Module["preRun"]=="function")Module["preRun"]=[Module["preRun"]];while(Module["preRun"].length){addOnPreRun(Module["preRun"].shift())}}callRuntimeCallbacks(onPreRuns)}function initRuntime(){runtimeInitialized=true;wasmExports["e"]()}function postRun(){if(Module["postRun"]){if(typeof Module["postRun"]=="function")Module["postRun"]=[Module["postRun"]];while(Module["postRun"].length){addOnPostRun(Module["postRun"].shift())}}callRuntimeCallbacks(onPostRuns)}function abort(what){Module["onAbort"]?.(what);what=`Aborted(${what})`;err(what);ABORT=true;what+=". Build with -sASSERTIONS for more info.";var e=new WebAssembly.RuntimeError(what);throw e}var wasmBinaryFile;function findWasmBinary(){return locateFile("kick_engine.wasm")}function getBinarySync(file){if(file==wasmBinaryFile&&wasmBinary){return new Uint8Array(wasmBinary)}if(readBinary){return readBinary(file)}throw"both async and sync fetching of the wasm failed"}async function getWasmBinary(binaryFile){if(!wasmBinary){try{var response=await readAsync(binaryFile);return new Uint8Array(response)}catch{}}return getBinarySync(binaryFile)}async function instantiateArrayBuffer(binaryFile,imports){try{var binary=await getWasmBinary(binaryFile);var instance=await WebAssembly.instantiate(binary,imports);return instance}catch(reason){err(`failed to asynchronously prepare wasm: ${reason}`);abort(reason)}}async function instantiateAsync(binary,binaryFile,imports){if(!binary){try{var response=fetch(binaryFile,{credentials:"same-origin"});var instantiationResult=await WebAssembly.instantiateStreaming(response,imports);return instantiationResult}catch(reason){err(`wasm streaming compile failed: ${reason}`);err("falling back to ArrayBuffer instantiation")}}return instantiateArrayBuffer(binaryFile,imports)}function getWasmImports(){var imports={a:wasmImports};return imports}async function createWasm(){function receiveInstance(instance,module){wasmExports=instance.exports;assignWasmExports(wasmExports);updateMemoryViews();return wasmExports}function receiveInstantiationResult(result){return receiveInstance(result["instance"])}var info=getWasmImports();if(Module["instantiateWasm"]){return new Promise((resolve,reject)=>{Module["instantiateWasm"](info,(inst,mod)=>{resolve(receiveInstance(inst,mod))})})}wasmBinaryFile??=findWasmBinary();var result=await instantiateAsync(wasmBinary,wasmBinaryFile,info);var exports=receiveInstantiationResult(result);return exports}class ExitStatus{name="ExitStatus";constructor(status){this.message=`Program terminated with exit(${status})`;this.status=status}}var HEAP16;var HEAP32;var HEAP64;var HEAP8;var HEAPF32;var HEAPF64;var HEAPU16;var HEAPU32;var HEAPU64;var HEAPU8;var callRuntimeCallbacks=callbacks=>{while(callbacks.length>0){callbacks.shift()(Module)}};var onPostRuns=[];var addOnPostRun=cb=>onPostRuns.push(cb);var onPreRuns=[];var addOnPreRun=cb=>onPreRuns.push(cb);var noExitRuntime=true;var __abort_js=()=>abort("");var _emscripten_date_now=()=>Date.now();var getHeapMax=()=>2147483648;var alignMemory=(size,alignment)=>Math.ceil(size/alignment)*alignment;var growMemory=size=>{var oldHeapSize=wasmMemory.buffer.byteLength;var pages=(size-oldHeapSize+65535)/65536|0;try{wasmMemory.grow(pages);updateMemoryViews();return 1}catch(e){}};var _emscripten_resize_heap=requestedSize=>{var oldSize=HEAPU8.length;requestedSize>>>=0;var maxHeapSize=getHeapMax();if(requestedSize>maxHeapSize){return false}for(var cutDown=1;cutDown<=4;cutDown*=2){var overGrownHeapSize=oldSize*(1+.2/cutDown);overGrownHeapSize=Math.min(overGrownHeapSize,requestedSize+100663296);var newSize=Math.min(maxHeapSize,alignMemory(Math.max(requestedSize,overGrownHeapSize),65536));var replacement=growMemory(newSize);if(replacement){return true}}return false};{if(Module["noExitRuntime"])noExitRuntime=Module["noExitRuntime"];if(Module["print"])out=Module["print"];if(Module["printErr"])err=Module["printErr"];if(Module["wasmBinary"])wasmBinary=Module["wasmBinary"];if(Module["arguments"])programArgs=Module["arguments"];if(Module["thisProgram"])thisProgram=Module["thisProgram"];if(Module["preInit"]){if(typeof Module["preInit"]=="function")Module["preInit"]=[Module["preInit"]];while(Module["preInit"].length>0){Module["preInit"].shift()()}}}var _engine_init,_engine_process,_engine_set_step,_engine_set_base,_engine_set_lane,_engine_set_bpm,_engine_set_reverb,_engine_play,_engine_stop,_engine_get_step,_engine_set_macro,_malloc,_free,memory,__indirect_function_table,wasmMemory;function assignWasmExports(wasmExports){_engine_init=Module["_engine_init"]=wasmExports["f"];_engine_process=Module["_engine_process"]=wasmExports["g"];_engine_set_step=Module["_engine_set_step"]=wasmExports["h"];_engine_set_base=Module["_engine_set_base"]=wasmExports["i"];_engine_set_lane=Module["_engine_set_lane"]=wasmExports["j"];_engine_set_bpm=Module["_engine_set_bpm"]=wasmExports["k"];_engine_set_reverb=Module["_engine_set_reverb"]=wasmExports["l"];_engine_play=Module["_engine_play"]=wasmExports["m"];_engine_stop=Module["_engine_stop"]=wasmExports["n"];_engine_get_step=Module["_engine_get_step"]=wasmExports["o"];_engine_set_macro=Module["_engine_set_macro"]=wasmExports["p"];_malloc=Module["_malloc"]=wasmExports["q"];_free=Module["_free"]=wasmExports["r"];memory=wasmMemory=wasmExports["d"];__indirect_function_table=wasmExports["__indirect_function_table"]}var wasmImports={a:__abort_js,c:_emscripten_date_now,b:_emscripten_resize_heap};async function run(){preRun();var setStatus=Module["setStatus"];if(setStatus){setStatus("Running...");await new Promise(resolve=>setTimeout(resolve,1));setTimeout(setStatus,1,"")}if(ABORT)return;initRuntime();Module["onRuntimeInitialized"]?.();postRun()}var wasmExports;wasmExports=await createWasm();await run();
;return Module}})();if(typeof exports==="object"&&typeof module==="object"){module.exports=KickEngineModule;module.exports.default=KickEngineModule}else if(typeof define==="function"&&define["amd"])define([],()=>KickEngineModule);

// kick-worklet.js — concatenated after kick_engine.js in kick-worklet-bundle.js
// KickEngineModule and __kickBase are in scope.
// WASM bytes are sent from the main thread via 'init' message (ArrayBuffer).

let _mod = null, _outLPtr = 0, _outRPtr = 0;
const _lanePtr = [0, 0, 0];
let _ready = false;
const _pending = [];
let _port = null;

function initModule(sr, wasmBuf) {
    const report = msg => { if (_port) _port.postMessage({ type: 'status', msg }); };
    report('initModule sr=' + sr + ' bytes=' + (wasmBuf && wasmBuf.byteLength));

    KickEngineModule({
        instantiateWasm: (imports, receiveInstance) => {
            WebAssembly.instantiate(wasmBuf, imports)
                .then(r  => { report('wasm ok'); receiveInstance(r.instance); })
                .catch(e => report('ERROR: ' + e));
            return {};
        }
    }).then(m => {
        _mod = m;
        _outLPtr = m._malloc(128 * 4);
        _outRPtr = m._malloc(128 * 4);
        for (let v = 0; v < 3; v++) _lanePtr[v] = m._malloc(128);
        m._engine_init(sr);
        _ready = true;
        report('ready, dispatching ' + _pending.length + ' msgs');
        for (const d of _pending) dispatch(_mod, d);
        _pending.length = 0;
    }).catch(e => { if (_port) _port.postMessage({ type: 'status', msg: 'MODULE ERROR: ' + e }); });
}

function dispatch(m, d) {
    switch (d.type) {
        case 'set_step':   m._engine_set_step(d.v, d.step, d.active ? 1 : 0, d.vel); break;
        case 'set_base':   m._engine_set_base(d.v, d.param, d.value); break;
        case 'set_lane': {
            const ptr = _lanePtr[d.v];
            m.HEAPU8.set(new Uint8Array(d.buf), ptr);
            m._engine_set_lane(d.v, ptr);
            break;
        }
        case 'set_bpm':    m._engine_set_bpm(d.bpm); break;
        case 'set_reverb': m._engine_set_reverb(d.decay, d.lowPass, d.preDelay, d.ret); break;
        case 'set_macro':  m._engine_set_macro(d.value); break;
        case 'play':       m._engine_play(); break;
        case 'stop':       m._engine_stop(); break;
    }
}

class KickProcessor extends AudioWorkletProcessor {
    constructor() {
        super();
        this._lastStep = -1;
        if (!_port) _port = this.port;
        this.port.onmessage = ({ data }) => {
            if (data.type === 'init') {
                initModule(data.sr, data.wasmBuf);
            } else if (_ready) {
                dispatch(_mod, data);
            } else {
                _pending.push(data);
            }
        };
    }

    process(_inputs, outputs) {
        if (!_ready) return true;
        _mod._engine_process(_outLPtr, _outRPtr, 128);
        const lOff = _outLPtr >>> 2, rOff = _outRPtr >>> 2;
        const heap = _mod.HEAPF32;
        const ch = outputs[0];
        ch[0].set(heap.subarray(lOff, lOff + 128));
        if (ch[1]) ch[1].set(heap.subarray(rOff, rOff + 128));
        const step = _mod._engine_get_step();
        if (step !== this._lastStep) {
            this._lastStep = step;
            this.port.postMessage({ type: 'step', step });
        }
        return true;
    }
}

registerProcessor('kick-processor', KickProcessor);
