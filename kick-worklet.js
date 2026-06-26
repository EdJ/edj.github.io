// kick-worklet.js — AudioWorklet processor body.
// Concatenated after kick_engine.js by build.sh → kick-worklet-bundle.js.
// KickEngineModule and __kickBase are already in scope when this runs.
//
// WASM is NOT fetched here. The main thread pre-compiles kick_engine.wasm
// (where fetch() works normally) and sends the WebAssembly.Module via the
// 'init' message. This sidesteps Emscripten's broken environment detection
// in AudioWorkletGlobalScope (github.com/emscripten-core/emscripten/issues/6230).

let _mod = null, _outLPtr = 0, _outRPtr = 0;
const _lanePtr = [0, 0, 0];
let _ready = false;
const _pending = [];

// _port is set from the first KickProcessor instance so initModule can report status.
let _port = null;

function initModule(sr, wasmBuf) {
    const report = (msg) => { if (_port) _port.postMessage({ type: 'status', msg }); };

    report('initModule called, wasmBuf byteLength=' + (wasmBuf && wasmBuf.byteLength));

    KickEngineModule({
        // Provide bytes directly — avoids WebAssembly.Module postMessage issues.
        // instantiate(bytes) returns {instance, module}; pass instance to Emscripten.
        instantiateWasm: (imports, receiveInstance) => {
            WebAssembly.instantiate(wasmBuf, imports)
                .then(r  => { report('WASM instance OK'); receiveInstance(r.instance); })
                .catch(e => report('ERROR instantiate: ' + e));
            return {};
        }
    }).then(m => {
        _mod = m;
        _outLPtr = m._malloc(128 * 4);
        _outRPtr = m._malloc(128 * 4);
        for (let v = 0; v < 3; v++) _lanePtr[v] = m._malloc(128);
        m._engine_init(sr);
        _ready = true;
        report('ready — dispatching ' + _pending.length + ' queued msgs');
        for (const d of _pending) dispatch(_mod, d);
        _pending.length = 0;
    }).catch(e => report('ERROR KickEngineModule: ' + e));
}

function dispatch(m, d) {
    switch (d.type) {
        case 'set_step':   m._engine_set_step(d.v, d.step, d.active ? 1 : 0, d.vel); break;
        case 'set_base':   m._engine_set_base(d.v, d.param, d.value); break;
        case 'set_mod':    m._engine_set_mod_amount(d.v, d.param, d.src, d.amount); break;
        case 'set_lane': {
            const ptr = _lanePtr[d.v];
            m.HEAPU8.set(new Uint8Array(d.buf), ptr);
            m._engine_set_lane(d.v, ptr);
            break;
        }
        case 'set_lfo':    m._engine_set_lfo(d.v, d.lfoType, d.rateBeats, d.depth); break;
        case 'set_bpm':    m._engine_set_bpm(d.bpm); break;
        case 'set_reverb': m._engine_set_reverb(d.decay, d.lowPass, d.preDelay, d.ret); break;
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
        const lOff = _outLPtr >>> 2;
        const rOff = _outRPtr >>> 2;
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
