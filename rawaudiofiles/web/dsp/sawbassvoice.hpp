// sawbassvoice.hpp — single PolyBLEP saw bass with LP filter and overdrive
#pragma once
#include <cmath>
#include <algorithm>
#include "fastsvf.hpp"
#include "overdrive.hpp"
#include "lib/envelopes.hpp"

class SawBassVoice {
public:
    static constexpr int SB_AMP_DECAY  = 0;
    static constexpr int SB_LP_CUTOFF  = 1;
    static constexpr int SB_DRIVE      = 2;
    static constexpr int SB_GAIN       = 3;
    static constexpr int SB_NUM_PARAMS = 4;
    static constexpr int NUM_STEPS     = 16;
    static constexpr int BUF           = 24;

    float param[SB_NUM_PARAMS] = { 0.35f, 0.45f, 0.0f, 0.50f };
    bool  steps[NUM_STEPS]     = {};
    float stepVel[NUM_STEPS];
    int   stepNote[NUM_STEPS];

    SawBassVoice() {
        for (int i = 0; i < NUM_STEPS; ++i) {
            stepVel[i]  = 0.75f;
            stepNote[i] = 38; // D2
        }
    }

    void init(int sr) {
        sampleRate       = static_cast<float>(sr);
        srInv            = 1.0f / sampleRate;
        env.samplesPerMs = sampleRate / 1000.0f;
        lp.Init();
        updateLP();
        updateDecay();
    }

    void setParam(int p, float v) {
        if (p < 0 || p >= SB_NUM_PARAMS) return;
        param[p] = v;
        if (p == SB_AMP_DECAY) updateDecay();
        if (p == SB_LP_CUTOFF) updateLP();
        if (p == SB_DRIVE)     overdrive.update(v);
    }

    void triggerStep(int step) {
        if (!steps[step]) return;
        baseHz      = noteToHz(stepNote[step]);
        velocity    = stepVel[step];
        pendingTrig = true;
    }

    bool active() const { return env.state > 0.0f || pendingTrig; }

    void process(float* outL, float* outR, float* /*rvL*/, float* /*rvR*/, int n) {
        if (!active()) return;
        env.process(pendingTrig, n);
        pendingTrig = false;

        const float gain = param[SB_GAIN];
        const float dt   = baseHz * srInv;
        float tmp[BUF];

        for (int i = 0; i < n; ++i) {
            phase += dt;
            if (phase >= 1.0f) phase -= 1.0f;
            tmp[i] = lp.process_low(sawPolyBlep(phase, dt) * env.buffer[i] * velocity * gain);
        }

        if (overdrive.amount > 0.001f)
            overdrive.processBuffer(tmp, n);

        for (int i = 0; i < n; ++i) {
            outL[i] += tmp[i];
            outR[i] += tmp[i];
        }
    }

private:
    float sampleRate = 48000.0f, srInv = 1.0f / 48000.0f;
    float baseHz     = 73.42f; // D2
    float velocity   = 1.0f;
    float phase      = 0.0f;
    bool  pendingTrig = false;
    DecayEnv<EnvType::Amp, BUF> env;
    FastSvf lp;
    QuickOverdrive overdrive;

    inline float sawPolyBlep(float phase, float dt) const {
        float s = 2.0f * phase - 1.0f;
        if (phase < dt) {
            float t = phase / dt;
            s -= t + t - t * t - 1.0f;
        } else if (phase > 1.0f - dt) {
            float t = (phase - 1.0f) / dt;
            s -= t * t + t + t + 1.0f;
        }
        return s;
    }

    static float noteToHz(int n) {
        return 440.0f * std::pow(2.0f, static_cast<float>(n - 69) / 12.0f);
    }

    void updateDecay() {
        env.setDecay(static_cast<int>(40.0f + param[SB_AMP_DECAY] * 2500.0f));
    }

    void updateLP() {
        float f = 20.0f * std::pow(1200.0f, param[SB_LP_CUTOFF]) / sampleRate;
        f = std::min(f, 0.499f);
        lp.set_f_q(f, 0.70f);
    }
};
