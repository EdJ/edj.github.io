// fmvoice.hpp — 2-op FM synth voice, portable C++17
// Mirrors the topology of Easter's Fm2Oscillator without Apple-framework deps.
// Uses Easter's DecayEnv<EnvType::Amp, 24> directly (already in lib/envelopes.hpp).

#pragma once
#include <cmath>
#include <cstring>
#include <algorithm>
#include "fastsvf.hpp"
#include "lib/envelopes.hpp"

class FmVoice {
public:
    static constexpr int FM_MOD_RATIO   = 0;
    static constexpr int FM_MOD_DEPTH   = 1;
    static constexpr int FM_FEEDBACK    = 2;
    static constexpr int FM_SUB_MIX     = 3;
    static constexpr int FM_AMP_DECAY   = 4;
    static constexpr int FM_LP_CUTOFF   = 5;
    static constexpr int FM_REVERB_SEND = 6;
    static constexpr int FM_GAIN        = 7;
    static constexpr int FM_NUM_PARAMS  = 8;
    static constexpr int NUM_STEPS      = 16;
    static constexpr int BUF            = 24; // matches FRAME_SIZE

    float param[FM_NUM_PARAMS] = {
        2.0f,  // modRatio
        0.04f, // modDepth
        0.0f,  // feedback
        0.0f,  // subMix
        0.35f, // ampDecay
        0.50f, // lpCutoff
        0.0f,  // reverbSend
        0.25f, // gain
    };

    bool  steps[NUM_STEPS]    = {};
    float stepVel[NUM_STEPS];
    int   stepNote[NUM_STEPS];

    FmVoice() {
        for (int i = 0; i < NUM_STEPS; ++i) {
            stepVel[i]  = 0.75f;
            stepNote[i] = 36;
        }
    }

    void init(int sr) {
        sampleRate        = static_cast<float>(sr);
        srInv             = 1.0f / sampleRate;
        env.samplesPerMs  = sampleRate / 1000.0f;
        lp.Init();
        updateLP();
        updateDecay();
        initSineTable();
    }

    void setParam(int p, float v) {
        if (p < 0 || p >= FM_NUM_PARAMS) return;
        param[p] = v;
        if (p == FM_LP_CUTOFF) updateLP();
        if (p == FM_AMP_DECAY) updateDecay();
    }

    void triggerStep(int step) {
        if (!steps[step]) return;
        float hz     = noteToHz(stepNote[step]);
        carHz        = hz;
        modHz        = hz * param[FM_MOD_RATIO];
        subHz        = hz * 0.5f;
        velocity     = stepVel[step];
        pendingTrig  = true;
    }

    bool active() const { return env.state > 0.0f || pendingTrig; }

    void process(float* outL, float* outR, float* rvL, float* rvR, int n) {
        if (!active()) return;

        // Easter DecayEnv<Amp>::process — fills env.buffer[0..n-1] with cubic-shaped decay
        env.process(pendingTrig, n);
        pendingTrig = false;

        float depth = param[FM_MOD_DEPTH] * 8.0f * 3.14159265f;
        float fb    = param[FM_FEEDBACK]  * 4.0f;
        float sub   = param[FM_SUB_MIX];
        float send  = param[FM_REVERB_SEND];
        float gain  = param[FM_GAIN];

        for (int i = 0; i < n; ++i) {
            float mod = fastSin(modPhase);
            float fbv = fb * fbPrev;
            float car = fastSin(carPhase + depth * mod + fbv);
            fbPrev = car;
            float s   = fastSin(subPhase);

            float sig = (car + s * sub) * env.buffer[i] * velocity * gain;
            sig = lp.process_low(sig);

            outL[i] += sig;
            outR[i] += sig;
            rvL[i]  += sig * send;
            rvR[i]  += sig * send;

            carPhase += carHz * srInv; if (carPhase >= 1.0f) carPhase -= 1.0f;
            modPhase += modHz * srInv; if (modPhase >= 1.0f) modPhase -= 1.0f;
            subPhase += subHz * srInv; if (subPhase >= 1.0f) subPhase -= 1.0f;
        }
    }

private:
    float sampleRate = 48000.0f;
    float srInv      = 1.0f / 48000.0f;
    float carHz      = 220.0f;
    float modHz      = 440.0f;
    float subHz      = 110.0f;
    float velocity   = 1.0f;
    float carPhase   = 0.0f;
    float modPhase   = 0.1f;
    float subPhase   = 0.2f;
    float fbPrev     = 0.0f;
    bool  pendingTrig = false;
    DecayEnv<EnvType::Amp, BUF> env;
    FastSvf lp;

    void updateDecay() {
        env.setDecay(static_cast<int>(40.0f + param[FM_AMP_DECAY] * 2500.0f));
    }

    void updateLP() {
        float f = 20.0f * std::pow(1200.0f, param[FM_LP_CUTOFF]) / sampleRate;
        f = std::min(f, 0.499f);
        lp.set_f_q(f, 0.65f);
    }

    static float noteToHz(int n) {
        return 440.0f * std::pow(2.0f, static_cast<float>(n - 69) / 12.0f);
    }

    static constexpr int SINE_LEN = 1024;
    static float s_sine[SINE_LEN + 1];
    static bool  s_sineReady;

    static void initSineTable() {
        if (s_sineReady) return;
        for (int i = 0; i <= SINE_LEN; ++i)
            s_sine[i] = std::sin(i * (2.0f * 3.14159265f) / SINE_LEN);
        s_sineReady = true;
    }

    inline float fastSin(float phase) const {
        float idx = phase * SINE_LEN;
        int   i   = static_cast<int>(idx) & (SINE_LEN - 1);
        float t   = idx - static_cast<float>(static_cast<int>(idx));
        return s_sine[i] + t * (s_sine[i + 1] - s_sine[i]);
    }
};

inline float FmVoice::s_sine[FmVoice::SINE_LEN + 1] = {};
inline bool  FmVoice::s_sineReady = false;
