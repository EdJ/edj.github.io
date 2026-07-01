// kick-worklet.js — concatenated after kick_engine.js in kick-worklet-bundle.js
// KickEngineModule and __kickBase are in scope.
// WASM bytes are sent from the main thread via 'init' message (ArrayBuffer).

let _mod = null, _outLPtr = 0, _outRPtr = 0, _sampInLPtr = 0, _sampInRPtr = 0;
const _lanePtr = [0, 0, 0];
let _ready = false;
const _pending = [];
let _port = null;

// ── Sampler ───────────────────────────────────────────────────────────────────
let _sampleL = null, _sampleR = null, _sampleLen = 0;
const _sliceOffsets = [0, 0, 0];
const _samplerSteps = new Int8Array(16).fill(-1); // -1=off, 0/1/2=slice index
let _samplerPos = -1;
let _samplerGain = 0.7;
let _samplerEnv = 1.0;
let _samplerFade = 0; // per-sample env decrement; 0 = no fade

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
        _sampInLPtr = m._engine_sampler_in_l();
        _sampInRPtr = m._engine_sampler_in_r();
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
        case 'set_macro':    m._engine_set_macro(d.value); break;
        case 'set_swing':    m._engine_set_swing(d.amount); break;
        case 'set_fm_step':    m._engine_set_fm_step(d.voice, d.step, d.active ? 1 : 0, d.note, d.vel); break;
        case 'set_fm_param':   m._engine_set_fm_param(d.voice, d.param, d.value); break;
        case 'set_swarm_step': m._engine_set_swarm_step(d.step, d.active ? 1 : 0, d.note, d.vel); break;
        case 'set_swarm_param':m._engine_set_swarm_param(d.param, d.value); break;
        case 'play':       m._engine_play(); break;
        case 'stop':
            m._engine_stop();
            if (_samplerPos >= 0) {
                _samplerFade = 1.0 / (48000 * 0.12); // 120 ms fade
                if (_port) _port.postMessage({ type: 'sampler_stop' });
            }
            break;
        case 'sampler_load': {
            _sampleL = new Float32Array(d.left);
            _sampleR = d.right ? new Float32Array(d.right) : null;
            _sampleLen = _sampleL.length;
            _sliceOffsets[0] = d.slices[0];
            _sliceOffsets[1] = d.slices[1];
            _sliceOffsets[2] = d.slices[2];
            break;
        }
        case 'sampler_set_step':   _samplerSteps[d.step] = d.slice; break;
        case 'sampler_gain':       _samplerGain = d.value; break;
        case 'sampler_set_slices':
            _sliceOffsets[0] = d.slices[0];
            _sliceOffsets[1] = d.slices[1];
            _sliceOffsets[2] = d.slices[2];
            break;
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

        // Write sampler into WASM input buffers so the engine routes it through FX.
        // Always fill all 128 slots — zeros for any positions past the end of the sample —
        // because the C++ engine reads samplerInL/R in FRAME_SIZE chunks across multiple
        // processFrame() calls and needs the full block to be present.
        if (_sampInLPtr) {
            const heap = _mod.HEAPF32;
            const lOff = _sampInLPtr >>> 2;
            const rOff = _sampInRPtr >>> 2;
            for (let i = 0; i < 128; i++) {
                if (_samplerPos >= 0 && _samplerPos < _sampleLen) {
                    const env = _samplerEnv;
                    const g = _samplerGain;
                    heap[lOff + i] = _sampleL[_samplerPos] * g * env;
                    heap[rOff + i] = (_sampleR ? _sampleR[_samplerPos] : _sampleL[_samplerPos]) * g * env;
                    _samplerPos++;
                    if (_samplerFade > 0) {
                        _samplerEnv -= _samplerFade;
                        if (_samplerEnv <= 0) { _samplerEnv = 0; _samplerPos = -1; }
                    } else if (_samplerPos >= _sampleLen) {
                        if (_port) _port.postMessage({ type: 'sampler_stop' });
                        _samplerPos = -1;
                    }
                } else {
                    heap[lOff + i] = 0;
                    heap[rOff + i] = 0;
                }
            }
        }

        if (_sampInLPtr) _mod._engine_sampler_push(128);
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
            if (_sampleL && step >= 0 && step < 16 && _samplerSteps[step] >= 0) {
                _samplerPos = _sliceOffsets[_samplerSteps[step]];
                _samplerEnv = 1.0;
                _samplerFade = 0;
                if (_port) _port.postMessage({ type: 'sampler_start', offset: _samplerPos });
            }
        }

        return true;
    }
}

registerProcessor('kick-processor', KickProcessor);
