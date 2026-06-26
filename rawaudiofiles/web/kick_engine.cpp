#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdint>

#include "dsp/vdsp.hpp"
#include "dsp/lib/tables.hpp"
#include "dsp/kick_web.hpp"
#include "dsp/lfo.hpp"
#include "dsp/reverb.hpp"

static constexpr int FRAME_SIZE = 24;
static constexpr int NUM_VOICES = 3;
static constexpr int NUM_STEPS  = 16;
static constexpr int LANE_SIZE  = 128;

// Matches the mod destination indices exposed to JS
enum KickParam {
    SubGain = 0, SubDecay,
    TransientGain, TransientDecay,
    NoiseGain, NoiseDecay,
    NoiseLowPass, NoiseHighPass,
    ReverbSend,
    NUM_PARAMS
};

enum ModSrc { Velocity = 0, LfoSrc, LaneSrc, NUM_MOD_SOURCES };

struct VoiceState {
    Kick    kick;
    Lfo     lfo;
    float   base[NUM_PARAMS]                    = {};
    float   modAmt[NUM_PARAMS][NUM_MOD_SOURCES] = {};
    uint8_t lane[LANE_SIZE]                     = {};
    bool    steps[NUM_STEPS]                    = {};
    float   stepVel[NUM_STEPS]                  = {};
    float   tempL[FRAME_SIZE]                   = {};
    float   tempR[FRAME_SIZE]                   = {};

    explicit VoiceState(int sr) : kick(sr) {
        lfo.setSampleRate(static_cast<float>(sr));
        lfo.frameSize_            = FRAME_SIZE;
        lfo.settings_.type        = LfoType::Sin;
        lfo.settings_.rateBeats   = 8;
        lfo.settings_.depth       = 1.0f;

        base[SubGain]        = 1.0f;
        base[SubDecay]       = 0.4f;
        base[TransientGain]  = 0.5f;
        base[TransientDecay] = 0.1f;
        base[NoiseGain]      = 0.5f;
        base[NoiseDecay]     = 0.2f;
        base[NoiseLowPass]   = 1.0f;
        base[NoiseHighPass]  = 0.0125f;
        base[ReverbSend]     = 0.0f;

        for (int s = 0; s < NUM_STEPS; ++s) stepVel[s] = 0.75f;
        for (int i = 0; i < LANE_SIZE;  ++i) lane[i]   = 128;
    }
};

class KickEngine {
public:
    explicit KickEngine(int sr)
        : sampleRate(sr), reverb(static_cast<float>(sr))
        , v0(sr), v1(sr), v2(sr)
    {
        voices[0] = &v0;
        voices[1] = &v1;
        voices[2] = &v2;

        // Default patterns: 4-on-the-floor, offbeat, half-time accent
        for (int s : {0,4,8,12}) voices[0]->steps[s] = true;
        for (int s : {2,6,10,14}) voices[1]->steps[s] = true;
        voices[2]->steps[0] = voices[2]->steps[8] = true;

        recomputeStepDuration();
        reverb.setDefaultParameters();
    }

    void process(float* outL, float* outR, int frameCount) {
        int pos = 0;
        if (samplesToProcess != FRAME_SIZE) {
            int remaining = FRAME_SIZE - samplesToProcess;
            int toCopy    = std::min(remaining, frameCount);
            memcpy(outL, voiceBufL + samplesToProcess, toCopy * sizeof(float));
            memcpy(outR, voiceBufR + samplesToProcess, toCopy * sizeof(float));
            pos += toCopy;
            samplesToProcess += toCopy;
            if (samplesToProcess < FRAME_SIZE) return;
        }
        while (pos < frameCount) {
            processFrame();
            samplesToProcess = std::min(FRAME_SIZE, frameCount - pos);
            memcpy(outL + pos, voiceBufL, samplesToProcess * sizeof(float));
            memcpy(outR + pos, voiceBufR, samplesToProcess * sizeof(float));
            pos += FRAME_SIZE;
        }
    }

    void setStep(int v, int step, bool active, float vel) {
        if (v < 0 || v >= NUM_VOICES || step < 0 || step >= NUM_STEPS) return;
        voices[v]->steps[step]  = active;
        voices[v]->stepVel[step]= vel;
    }
    void setBase(int v, int param, float value) {
        if (v < 0 || v >= NUM_VOICES || param < 0 || param >= NUM_PARAMS) return;
        voices[v]->base[param] = value;
    }
    void setModAmount(int v, int param, int src, float amount) {
        if (v < 0 || v >= NUM_VOICES || param < 0 || param >= NUM_PARAMS
            || src < 0 || src >= NUM_MOD_SOURCES) return;
        voices[v]->modAmt[param][src] = amount;
    }
    void setLane(int v, const uint8_t* data) {
        if (v < 0 || v >= NUM_VOICES) return;
        memcpy(voices[v]->lane, data, LANE_SIZE);
    }
    void setLfo(int v, int type, int rateBeats, float depth) {
        if (v < 0 || v >= NUM_VOICES) return;
        voices[v]->lfo.settings_.type      = static_cast<LfoType>(type);
        voices[v]->lfo.settings_.rateBeats = rateBeats;
        voices[v]->lfo.settings_.depth     = depth;
        voices[v]->lfo.updateBpm(static_cast<int>(bpm));
    }
    void setBpm(float newBpm) {
        bpm = newBpm;
        recomputeStepDuration();
        for (int v = 0; v < NUM_VOICES; ++v)
            voices[v]->lfo.updateBpm(static_cast<int>(bpm));
    }
    void setReverb(float decay, float lowPass, float preDelay, float ret) {
        reverb.setDecay(decay);
        reverb.setLowPass(lowPass);
        reverb.setPreDelay(preDelay);
        reverbReturn = ret;
    }
    void play() {
        isPlaying    = true;
        currentStep  = -1;
        nextStepTime = 0;
        sampleTime   = 0;
        samplesToProcess = FRAME_SIZE;
        for (int v = 0; v < NUM_VOICES; ++v) voices[v]->lfo.reset();
        reverb.clear();
    }
    void stop() { isPlaying = false; }
    int getCurrentStep() const { return currentStep; }

private:
    int      sampleRate;
    float    bpm              = 120.f;
    bool     isPlaying        = false;
    int64_t  sampleTime       = 0;
    int64_t  nextStepTime     = 0;
    int      currentStep      = -1;
    int      samplesToProcess = FRAME_SIZE;
    float    stepDurSamples   = 0.f;
    float    reverbReturn     = 0.5f;

    VoiceState  v0, v1, v2;
    VoiceState* voices[NUM_VOICES];
    PlateReverb reverb;

    float voiceBufL[FRAME_SIZE]  = {};
    float voiceBufR[FRAME_SIZE]  = {};
    float reverbBufL[FRAME_SIZE] = {};
    float reverbBufR[FRAME_SIZE] = {};

    void recomputeStepDuration() {
        stepDurSamples = (60.f / bpm) * static_cast<float>(sampleRate) / 4.f;
    }

    float laneValue(int v) const {
        int step = std::max(currentStep, 0);
        float pos = (static_cast<float>(step) / NUM_STEPS) * LANE_SIZE;
        int i0 = static_cast<int>(pos) & (LANE_SIZE - 1);
        int i1 = (i0 + 1) & (LANE_SIZE - 1);
        float w = pos - static_cast<float>(i0);
        float a = static_cast<float>(voices[v]->lane[i0]) / 255.f;
        float b = static_cast<float>(voices[v]->lane[i1]) / 255.f;
        return a + w * (b - a);
    }

    float resolve(int v, KickParam p, float vel, float laneVal) const {
        float val = voices[v]->base[p];
        val += voices[v]->modAmt[p][Velocity] * vel;
        val += voices[v]->modAmt[p][LfoSrc]   * voices[v]->lfo.value;
        val += voices[v]->modAmt[p][LaneSrc]  * laneVal;
        return std::clamp(val, 0.f, 1.f);
    }

    void applyKickParams(int v, float vel, float laneVal) {
        auto& kick = voices[v]->kick;
        kick.update(
            static_cast<int>(resolve(v, SubDecay,       vel, laneVal) * 790.f + 10.f),
            static_cast<int>(resolve(v, TransientDecay, vel, laneVal) * 195.f +  5.f),
            static_cast<int>(resolve(v, NoiseDecay,     vel, laneVal) * 798.f +  2.f),
            true
        );
        kick.subAmp       = Tables::getValue<Tables::exponentialTable>(resolve(v, SubGain,       vel, laneVal));
        kick.transientAmp = Tables::getValue<Tables::exponentialTable>(resolve(v, TransientGain, vel, laneVal));
        kick.noiseAmp     = Tables::getValue<Tables::exponentialTable>(resolve(v, NoiseGain,     vel, laneVal)) * 0.35f;
        kick.totalAmp     = 1.0f;
        kick.updateNoiseSvf(
            Tables::getValue<Tables::filterNormalized>(resolve(v, NoiseHighPass, vel, laneVal)),
            Tables::getValue<Tables::filterNormalized>(resolve(v, NoiseLowPass,  vel, laneVal)),
            0.5f
        );
    }

    void processFrame() {
        // LFO advances every frame
        for (int v = 0; v < NUM_VOICES; ++v) voices[v]->lfo.process();

        if (isPlaying && sampleTime >= nextStepTime) {
            currentStep  = (currentStep + 1) % NUM_STEPS;
            nextStepTime = static_cast<int64_t>(
                static_cast<float>(nextStepTime) + stepDurSamples);

            for (int v = 0; v < NUM_VOICES; ++v) {
                float vel     = voices[v]->stepVel[currentStep];
                float laneVal = laneValue(v);
                applyKickParams(v, vel, laneVal);
                voices[v]->kick.trigger = voices[v]->steps[currentStep];
            }
        }

        memset(voiceBufL,  0, FRAME_SIZE * sizeof(float));
        memset(voiceBufR,  0, FRAME_SIZE * sizeof(float));
        memset(reverbBufL, 0, FRAME_SIZE * sizeof(float));
        memset(reverbBufR, 0, FRAME_SIZE * sizeof(float));

        for (int v = 0; v < NUM_VOICES; ++v) {
            auto& voice = *voices[v];
            voice.kick.process(voice.tempL, voice.tempR);
            voice.kick.trigger = false;

            float send = voice.base[ReverbSend];
            for (int i = 0; i < FRAME_SIZE; ++i) {
                voiceBufL[i]  += voice.tempL[i];
                voiceBufR[i]  += voice.tempR[i];
                reverbBufL[i] += voice.tempL[i] * send;
                reverbBufR[i] += voice.tempR[i] * send;
            }
        }

        if (reverbReturn > 0.f) {
            reverb.process(reverbBufL, reverbBufR, FRAME_SIZE);
            for (int i = 0; i < FRAME_SIZE; ++i) {
                voiceBufL[i] += reverbBufL[i] * reverbReturn;
                voiceBufR[i] += reverbBufR[i] * reverbReturn;
            }
        }

        sampleTime += FRAME_SIZE;
    }
};

// ── Global singleton ──────────────────────────────────────────────────────────

static KickEngine* g_engine = nullptr;

extern "C" {

void engine_init(int sampleRate) {
    delete g_engine;
    g_engine = new KickEngine(sampleRate);
}

void engine_process(float* outL, float* outR, int frameCount) {
    if (g_engine) g_engine->process(outL, outR, frameCount);
    else { memset(outL, 0, frameCount * 4); memset(outR, 0, frameCount * 4); }
}

void engine_set_step(int voice, int step, int active, float vel) {
    if (g_engine) g_engine->setStep(voice, step, active != 0, vel);
}

void engine_set_base(int voice, int param, float value) {
    if (g_engine) g_engine->setBase(voice, param, value);
}

void engine_set_mod_amount(int voice, int param, int src, float amount) {
    if (g_engine) g_engine->setModAmount(voice, param, src, amount);
}

void engine_set_lane(int voice, const uint8_t* data) {
    if (g_engine) g_engine->setLane(voice, data);
}

void engine_set_lfo(int voice, int type, int rateBeats, float depth) {
    if (g_engine) g_engine->setLfo(voice, type, rateBeats, depth);
}

void engine_set_bpm(float bpm) {
    if (g_engine) g_engine->setBpm(bpm);
}

void engine_set_reverb(float decay, float lowPass, float preDelay, float ret) {
    if (g_engine) g_engine->setReverb(decay, lowPass, preDelay, ret);
}

void engine_play() { if (g_engine) g_engine->play(); }
void engine_stop() { if (g_engine) g_engine->stop(); }
int  engine_get_step() { return g_engine ? g_engine->getCurrentStep() : -1; }

} // extern "C"
