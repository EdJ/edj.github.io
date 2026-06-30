#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdint>

#include "dsp/vdsp.hpp"
#include "dsp/lib/tables.hpp"
#include "dsp/kick_web.hpp"
#include "dsp/reverb.hpp"
#include "dsp/overdrive.hpp"
#include "dsp/sawbassvoice.hpp"
#include "dsp/swarmvoice.hpp"

static constexpr int FRAME_SIZE = 24;
static constexpr int NUM_VOICES = 3;
static constexpr int NUM_STEPS  = 16;
static constexpr int LANE_SIZE     = 128;

// Param indices — must match JS P.* constants
enum KickParam {
    SubGain = 0, SubDecay,
    TransientGain, TransientDecay,
    NoiseGain, NoiseDecay,
    NoiseLowPass, NoiseHighPass,
    NoiseResonance,   // was hardcoded 0.5; now exposed
    OverdriveAmt,     // QuickOverdrive drive amount
    ReverbSend,
    NUM_PARAMS
};

struct VoiceState {
    Kick           kick;
    QuickOverdrive overdrive{0.0f};
    float          base[NUM_PARAMS]   = {};
    uint8_t        lane[LANE_SIZE]    = {};
    bool           steps[NUM_STEPS]  = {};
    float          stepVel[NUM_STEPS]= {};
    float          tempL[FRAME_SIZE] = {};
    float          tempR[FRAME_SIZE] = {};

    explicit VoiceState(int sr) : kick(sr) {
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
        sawBass.init(sr);
        swarm.init(sr);
        for (int s = 0; s < NUM_STEPS; ++s) swarm.stepNote[s] = 60; // lead: C4

        // ── Kick (C2 = 65 Hz): transient + sub + short noise blast ───────────
        voices[0]->kick.updateNote(36);
        voices[0]->base[SubGain]        = 1.0f;
        voices[0]->base[SubDecay]       = 0.35f;
        voices[0]->base[TransientGain]  = 0.85f;
        voices[0]->base[TransientDecay] = 0.07f;
        voices[0]->base[NoiseGain]      = 0.25f;
        voices[0]->base[NoiseDecay]     = 0.06f;
        voices[0]->base[NoiseLowPass]   = 0.50f;
        voices[0]->base[NoiseHighPass]  = 0.20f;
        voices[0]->base[NoiseResonance] = 0.30f;
        voices[0]->base[OverdriveAmt]   = 0.0f;
        voices[0]->base[ReverbSend]     = 0.0f;
        for (int s : {0, 8}) voices[0]->steps[s] = true;

        // ── Hat: noise-only, high bandpass, lane drives per-step accent ───────
        voices[1]->kick.updateNote(60); // note unused (SubGain = 0)
        voices[1]->base[SubGain]        = 0.0f;
        voices[1]->base[SubDecay]       = 0.2f;
        voices[1]->base[TransientGain]  = 0.0f;
        voices[1]->base[TransientDecay] = 0.05f;
        voices[1]->base[NoiseGain]      = 0.9f;
        voices[1]->base[NoiseDecay]     = 0.08f;
        voices[1]->base[NoiseLowPass]   = 0.88f;
        voices[1]->base[NoiseHighPass]  = 0.65f;
        voices[1]->base[NoiseResonance] = 0.65f;
        voices[1]->base[OverdriveAmt]   = 0.0f;
        voices[1]->base[ReverbSend]     = 0.0f;
        for (int s : {0,2,4,6,8,10,12,14}) voices[1]->steps[s] = true;

        // ── Snare (C3 = 130 Hz): noise + sub, no transient ───────────────────
        voices[2]->kick.updateNote(48);
        voices[2]->base[SubGain]        = 0.45f;
        voices[2]->base[SubDecay]       = 0.14f;
        voices[2]->base[TransientGain]  = 0.0f;
        voices[2]->base[TransientDecay] = 0.05f;
        voices[2]->base[NoiseGain]      = 0.8f;
        voices[2]->base[NoiseDecay]     = 0.16f;
        voices[2]->base[NoiseLowPass]   = 0.62f;  // ~2.5kHz, below hat HP
        voices[2]->base[NoiseHighPass]  = 0.40f;  // ~500Hz, punchy crack range
        voices[2]->base[NoiseResonance] = 0.52f;  // more peaked
        voices[2]->base[OverdriveAmt]   = 0.0f;
        voices[2]->base[ReverbSend]     = 0.0f;
        for (int s : {4, 12}) voices[2]->steps[s] = true;

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
        voices[v]->steps[step]   = active;
        voices[v]->stepVel[step] = vel;
    }
    void setBase(int v, int param, float value) {
        if (v < 0 || v >= NUM_VOICES || param < 0 || param >= NUM_PARAMS) return;
        voices[v]->base[param] = value;
    }
    void setLane(int v, const uint8_t* data) {
        if (v < 0 || v >= NUM_VOICES) return;
        memcpy(voices[v]->lane, data, LANE_SIZE);
    }
    void setBpm(float newBpm) {
        bpm = newBpm;
        recomputeStepDuration();
    }
    void setReverb(float decay, float lowPass, float preDelay, float ret) {
        reverb.setDecay(decay);
        reverb.setLowPass(lowPass);
        // HP and diffusion fixed from Easter defaults: highPass=filterNorm(0.25), diffusion=0.9
        reverb.setHighPass(20.f * std::pow(1200.f, 0.25f) / static_cast<float>(sampleRate));
        reverb.setDiffusion(0.9f);
        reverb.setPreDelay(preDelay);
        reverbReturn = ret;
    }
    void setSwing(float amount) { swingAmount = amount; }

    void setFmStep(int step, bool active, int note, float vel) {
        if (step < 0 || step >= NUM_STEPS) return;
        sawBass.steps[step]    = active;
        sawBass.stepNote[step] = note;
        sawBass.stepVel[step]  = vel;
    }
    void setFmParam(int param, float value) {
        sawBass.setParam(param, value);
    }
    void setSwarmStep(int step, bool active, int note, float vel) {
        if (step < 0 || step >= NUM_STEPS) return;
        swarm.steps[step]    = active;
        swarm.stepNote[step] = note;
        swarm.stepVel[step]  = vel;
    }
    void setSwarmParam(int param, float value) { swarm.setParam(param, value); }

    void play() {
        isPlaying        = true;
        currentStep      = -1;
        nextStepTime     = 0;
        baseStepTime     = 0;
        sampleTime       = 0;
        samplesToProcess = FRAME_SIZE;
        reverb.clear();
    }
    void stop()  { isPlaying = false; }
    int  getCurrentStep() const { return currentStep; }
    void setMacro(float value) {
        macroValue = value;
        float n = 0.05f + value * 0.85f; // 0→~30Hz (bypass), 1→~12kHz (extreme sweep)
        float f = (20.f * std::pow(1200.f, n)) / static_cast<float>(sampleRate);
        macroHp.set_f_q(f, 3.0f); // q=3 matches Easter highPassQuad
    }

private:
    int      sampleRate;
    float    bpm              = 120.f;
    bool     isPlaying        = false;
    int64_t  sampleTime       = 0;
    int64_t  nextStepTime     = 0;
    int64_t  baseStepTime     = 0;
    int      currentStep      = -1;
    int      samplesToProcess = FRAME_SIZE;
    float    stepDurSamples   = 0.f;
    float    reverbReturn     = 0.0f;
    float    macroValue       = 0.0f;
    float    swingAmount      = 0.0f;
    FastSvf  macroHp;

    VoiceState  v0, v1, v2;
    VoiceState* voices[NUM_VOICES];
    SawBassVoice sawBass;
    SwarmVoice  swarm;
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

    void applyKickParams(int v, float vel) {
        auto& kick = voices[v]->kick;
        auto& vv   = *voices[v];
        int noiseDecayMs = static_cast<int>(vv.base[NoiseDecay] * 798.f + 2.f);
        // Hat: lane = open/closed, vel also scales decay (louder hit → longer tail)
        if (v == 1) {
            float laneMod    = laneValue(1) * 2.0f;       // lane 0.5→1.0x, 0→0x, 1→2.0x
            float velDecayMod = 0.3f + vel * 0.7f;        // vel 0.25→0.475x, vel 1.0→1.0x
            noiseDecayMs = std::max(2, static_cast<int>(noiseDecayMs * laneMod * velDecayMod));
        }
        kick.update(
            static_cast<int>(vv.base[SubDecay]       * 790.f + 10.f),
            static_cast<int>(vv.base[TransientDecay] * 195.f +  5.f),
            noiseDecayMs,
            true
        );
        kick.subAmp       = Tables::getValue<Tables::exponentialTable>(vv.base[SubGain])       * vel;
        kick.transientAmp = Tables::getValue<Tables::exponentialTable>(vv.base[TransientGain]) * vel;
        kick.noiseAmp     = Tables::getValue<Tables::exponentialTable>(vv.base[NoiseGain])     * vel * 0.35f;
        kick.totalAmp     = 1.0f;
        float reso = Tables::getValue<Tables::stmResonanceNormalized>(vv.base[NoiseResonance]);
        kick.updateNoiseSvf(
            Tables::getValue<Tables::filterNormalized>(vv.base[NoiseHighPass]),
            Tables::getValue<Tables::filterNormalized>(vv.base[NoiseLowPass]),
            reso
        );
        vv.overdrive.update(vv.base[OverdriveAmt]);
    }

    void processFrame() {
        if (isPlaying && sampleTime >= nextStepTime) {
            currentStep  = (currentStep + 1) % NUM_STEPS;
            baseStepTime = static_cast<int64_t>(
                static_cast<float>(baseStepTime) + stepDurSamples);
            int nextStep = (currentStep + 1) % NUM_STEPS;
            float swingOff = (swingAmount > 0.001f && (nextStep % 2 == 1))
                ? swingAmount * stepDurSamples * 0.33f : 0.0f;
            nextStepTime = baseStepTime + static_cast<int64_t>(swingOff);

            for (int v = 0; v < NUM_VOICES; ++v) {
                bool triggered = voices[v]->steps[currentStep];
                voices[v]->kick.trigger = triggered;
                if (triggered) applyKickParams(v, voices[v]->stepVel[currentStep]);
            }
            sawBass.triggerStep(currentStep);
            swarm.triggerStep(currentStep);
        }

        memset(voiceBufL,  0, FRAME_SIZE * sizeof(float));
        memset(voiceBufR,  0, FRAME_SIZE * sizeof(float));
        memset(reverbBufL, 0, FRAME_SIZE * sizeof(float));
        memset(reverbBufR, 0, FRAME_SIZE * sizeof(float));

        for (int v = 0; v < NUM_VOICES; ++v) {
            auto& voice = *voices[v];
            voice.kick.process(voice.tempL, voice.tempR);
            voice.kick.trigger = false;

            if (voice.overdrive.amount > 0.001f) {
                voice.overdrive.processBuffer(voice.tempL, FRAME_SIZE);
                voice.overdrive.processBuffer(voice.tempR, FRAME_SIZE);
            }

            float send = voice.base[ReverbSend];
            for (int i = 0; i < FRAME_SIZE; ++i) {
                voiceBufL[i]  += voice.tempL[i];
                voiceBufR[i]  += voice.tempR[i];
                reverbBufL[i] += voice.tempL[i] * send;
                reverbBufR[i] += voice.tempR[i] * send;
            }
        }

        sawBass.process(voiceBufL, voiceBufR, reverbBufL, reverbBufR, FRAME_SIZE);
        swarm.process(voiceBufL, voiceBufR, reverbBufL, reverbBufR, FRAME_SIZE);

        // Macro: HP sweep + dry→wet crossfade (fully wet + extreme HP at macro=1)
        if (macroValue > 0.001f) {
            macroHp.process_high(voiceBufL, FRAME_SIZE);
            macroHp.process_high(voiceBufR, FRAME_SIZE);
            // Full reverb send at macro=1
            for (int i = 0; i < FRAME_SIZE; ++i) {
                reverbBufL[i] += voiceBufL[i] * macroValue;
                reverbBufR[i] += voiceBufR[i] * macroValue;
            }
            // Fade dry signal to silence as macro reaches 1
            float dryGain = 1.0f - macroValue;
            for (int i = 0; i < FRAME_SIZE; ++i) {
                voiceBufL[i] *= dryGain;
                voiceBufR[i] *= dryGain;
            }
        }

        float effectiveRvRet = (macroValue > 0.001f)
            ? std::max(reverbReturn, macroValue)
            : reverbReturn;
        if (effectiveRvRet > 0.f) {
            reverb.process(reverbBufL, reverbBufR, FRAME_SIZE);
            for (int i = 0; i < FRAME_SIZE; ++i) {
                voiceBufL[i] += reverbBufL[i] * effectiveRvRet;
                voiceBufR[i] += reverbBufR[i] * effectiveRvRet;
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

void engine_set_lane(int voice, const uint8_t* data) {
    if (g_engine) g_engine->setLane(voice, data);
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
void engine_set_macro(float value) { if (g_engine) g_engine->setMacro(value); }
void engine_set_swing(float amount) { if (g_engine) g_engine->setSwing(amount); }
void engine_set_fm_step(int /*voice*/, int step, int active, int note, float vel) {
    if (g_engine) g_engine->setFmStep(step, active != 0, note, vel);
}
void engine_set_fm_param(int /*voice*/, int param, float value) {
    if (g_engine) g_engine->setFmParam(param, value);
}
void engine_set_swarm_step(int step, int active, int note, float vel) {
    if (g_engine) g_engine->setSwarmStep(step, active != 0, note, vel);
}
void engine_set_swarm_param(int param, float value) {
    if (g_engine) g_engine->setSwarmParam(param, value);
}

} // extern "C"
