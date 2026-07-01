// swarmvoice.hpp — detuned swarm/spread oscillator, portable C++17
// Conceptually mirrors Easter's WavetableOscillator spread mode without Apple vDSP/Accelerate.
// Uses Easter's AttackDecayEnv<EnvType::Amp, 24> directly (already in lib/envelopes.hpp).
// PolyBLEP saw replaces Easter's mip-mapped wavetable (same alias suppression, no Accelerate).

#pragma once
#include <cmath>
#include <algorithm>
#include "fastsvf.hpp"
#include "lib/envelopes.hpp"

class SwarmVoice {
public:
    static constexpr int SW_SPREAD  = 0;
    static constexpr int SW_DRIFT   = 1;
    static constexpr int SW_DECAY   = 2;
    static constexpr int SW_CUTOFF  = 3;
    static constexpr int SW_REVERB  = 4;
    static constexpr int SW_GAIN    = 5;
    static constexpr int SW_DELAY   = 6;
    static constexpr int SW_ENV_AMT = 7;
    static constexpr int SW_NUM_PARAMS = 8;
    static constexpr int NUM_STEPS  = 16;
    static constexpr int NUM_VOICES = 5;
    static constexpr int BUF        = 24; // matches FRAME_SIZE

    float param[SW_NUM_PARAMS] = { 0.45f, 0.35f, 0.25f, 0.55f, 0.0f, 0.65f, 0.0f, 0.0f };
    bool  steps[NUM_STEPS]    = {};
    float stepVel[NUM_STEPS];
    int   stepNote[NUM_STEPS];

    SwarmVoice() {
        for (int i = 0; i < NUM_STEPS; ++i) {
            stepVel[i]  = 0.75f;
            stepNote[i] = 60;
        }
        for (int v = 0; v < NUM_VOICES; ++v) {
            phases[v]      = v * (1.0f / NUM_VOICES);
            driftPhases[v] = v * (1.0f / NUM_VOICES);
        }
    }

    void init(int sr) {
        sampleRate       = static_cast<float>(sr);
        srInv            = 1.0f / sampleRate;
        env.samplesPerMs = sampleRate / 1000.0f;
        lpL.Init();
        lpR.Init();
        updateLP();
        updateEnvTimes();
        initSine();
    }

    void setParam(int p, float v) {
        if (p < 0 || p >= SW_NUM_PARAMS) return;
        param[p] = v;
        if (p == SW_CUTOFF) updateLP();
        if (p == SW_DECAY)  updateEnvTimes();
    }

    void triggerStep(int step) {
        if (!steps[step]) return;
        baseHz      = noteToHz(stepNote[step]);
        velocity    = stepVel[step];
        pendingTrig = true;
    }

    bool active() const {
        return env.mode != AttackDecayEnvState::Done || env.state > 0.0f || pendingTrig;
    }

    void process(float* outL, float* outR, float* rvL, float* rvR, int n) {
        if (!active()) return;

        // Easter AttackDecayEnv<Amp>::process — RampToAttack→Attack→Decay, cubic shaping
        env.process(pendingTrig, n);
        pendingTrig = false;

        float spreadCents = param[SW_SPREAD] * 15.0f;
        float driftDepth  = param[SW_DRIFT]  * 0.002f;
        float rvSend      = param[SW_REVERB];
        float gain        = param[SW_GAIN] * (1.0f / NUM_VOICES);
        float delaySend   = param[SW_DELAY];
        float delayFb     = delaySend * 0.55f;
        float envAmt      = param[SW_ENV_AMT];
        float baseCutoffF = std::min(20.0f * std::pow(1200.0f, param[SW_CUTOFF]) / sampleRate, 0.499f);

        for (int i = 0; i < n; ++i) {
            float sumL = 0.0f, sumR = 0.0f;

            for (int v = 0; v < NUM_VOICES; ++v) {
                driftPhases[v] += DRIFT_RATES[v] * srInv;
                if (driftPhases[v] >= 1.0f) driftPhases[v] -= 1.0f;

                float drift = fastSin(driftPhases[v]) * driftDepth * baseHz;
                float ratio = std::pow(2.0f, SPREAD_ST[v] * spreadCents * (1.0f / 1200.0f));
                float hz    = baseHz * ratio + drift;
                float dt    = hz * srInv;

                phases[v] += dt;
                if (phases[v] >= 1.0f) phases[v] -= 1.0f;

                float s = sawPolyBlep(phases[v], dt);
                sumL += s * PAN_L[v];
                sumR += s * PAN_R[v];
            }

            // Amp envelope → filter modulation
            float ef = std::min(baseCutoffF + env.buffer[i] * envAmt * 0.45f, 0.499f);
            lpL.set_f_q(ef, 0.65f);
            lpR.set_f_q(ef, 0.65f);

            float ampEnv = env.buffer[i] * velocity * gain;
            float sigL = lpL.process_low(sumL * ampEnv);
            float sigR = lpR.process_low(sumR * ampEnv);

            // Delay
            int rdPos = (delayPos + DELAY_BUF - DELAY_SAMP) & (DELAY_BUF - 1);
            float dL = delayBufL[rdPos];
            float dR = delayBufR[rdPos];
            delayBufL[delayPos] = sigL + dL * delayFb;
            delayBufR[delayPos] = sigR + dR * delayFb;
            delayPos = (delayPos + 1) & (DELAY_BUF - 1);

            float outSigL = sigL + dL * delaySend;
            float outSigR = sigR + dR * delaySend;

            outL[i] += outSigL;
            outR[i] += outSigR;
            rvL[i]  += outSigL * rvSend;
            rvR[i]  += outSigR * rvSend;
        }
    }

private:
    float sampleRate = 48000.0f, srInv = 1.0f / 48000.0f;
    float baseHz     = 440.0f;
    float velocity   = 1.0f;
    bool  pendingTrig = false;
    AttackDecayEnv<EnvType::Amp, BUF> env;
    FastSvf lpL, lpR;
    float phases[NUM_VOICES]      = {};
    float driftPhases[NUM_VOICES] = {};

    static constexpr int DELAY_BUF  = 8192;   // power-of-2 ring buffer
    static constexpr int DELAY_SAMP = 6000;   // ~125 ms @ 48 kHz (≈ 1/8th @ 120 BPM)
    float delayBufL[DELAY_BUF] = {};
    float delayBufR[DELAY_BUF] = {};
    int   delayPos = 0;

    static constexpr float DRIFT_RATES[NUM_VOICES] = { 0.047f, 0.059f, 0.071f, 0.037f, 0.083f };
    static constexpr float SPREAD_ST[NUM_VOICES]   = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f };
    static constexpr float PAN_L[NUM_VOICES] = { 1.0f, 0.75f, 0.60f, 0.40f, 0.15f };
    static constexpr float PAN_R[NUM_VOICES] = { 0.15f, 0.40f, 0.60f, 0.75f, 1.0f };

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

    static constexpr int SINE_LEN = 1024;
    static float s_sine[SINE_LEN + 1];
    static bool  s_ready;

    static void initSine() {
        if (s_ready) return;
        for (int i = 0; i <= SINE_LEN; ++i)
            s_sine[i] = std::sin(i * (2.0f * 3.14159265f / SINE_LEN));
        s_ready = true;
    }

    inline float fastSin(float phase) const {
        float idx = phase * SINE_LEN;
        int   i   = static_cast<int>(idx) & (SINE_LEN - 1);
        float t   = idx - static_cast<float>(static_cast<int>(idx));
        return s_sine[i] + t * (s_sine[i + 1] - s_sine[i]);
    }

    static float noteToHz(int n) {
        return 440.0f * std::pow(2.0f, static_cast<float>(n - 69) / 12.0f);
    }

    void updateEnvTimes() {
        // 3ms attack (Easter's minimum is 1.5ms); decay scales with param
        env.update(3, static_cast<int>(40.0f + param[SW_DECAY] * 3000.0f));
    }

    void updateLP() {
        float f = 20.0f * std::pow(1200.0f, param[SW_CUTOFF]) / sampleRate;
        f = std::min(f, 0.499f);
        lpL.set_f_q(f, 0.65f);
        lpR.set_f_q(f, 0.65f);
    }
};

inline float SwarmVoice::s_sine[SwarmVoice::SINE_LEN + 1] = {};
inline bool  SwarmVoice::s_ready = false;
