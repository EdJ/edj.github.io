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
