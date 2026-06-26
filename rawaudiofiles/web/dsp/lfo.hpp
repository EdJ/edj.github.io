#pragma once

#include <cmath>
#include <algorithm>
#include <random>
#include "lib/tables.hpp"

enum LfoType : int { Sin = 0, Ramp, Square, Random, RandomWalk, LFOTYPE_COUNT };
enum SyncType : int { Beats = 0, Ms, SYNCTYPE_COUNT };

struct LfoSettings {
    float rateFactor    = 0.5f;
    int   rateMultiplier= 10;
    int   rateBeats     = 8;
    int   syncType      = SyncType::Beats;
    int   type          = LfoType::Sin;
    float depth         = 1.0f;
    float offset        = 0.0f;
    bool  reset         = false;
};

class Lfo {
public:
    float       sampleRate_ = 48000.0f;
    int         frameSize_  = 24;
    int         bpm_        = 120;
    LfoSettings settings_;
    bool        justReset_  = false;
    float       phaseIncrement_ = 0.0f;
    float       randomValue_    = 0.5f;
    float       walkSize_       = 0.15f;
    float       state = 0.0f;
    float       value = 0.0f;

    inline void updatePhaseIncrement() {
        int rateMs = (settings_.syncType == SyncType::Beats)
            ? static_cast<int>((60000.0f / bpm_) * settings_.rateBeats)
            : static_cast<int>(250 * settings_.rateMultiplier * settings_.rateFactor);
        float frequency = 1000.0f / static_cast<float>(rateMs);
        phaseIncrement_ = (frequency * 2.f * M_PI * static_cast<float>(frameSize_)) / sampleRate_;
    }

    static inline float clamp01(float v) { return std::clamp(v, -1.f, 1.f); }
    inline float applyDepthAndOffset(float v) const {
        return clamp01((v * settings_.depth) + settings_.offset);
    }
    inline static float randFloat(float a, float b) {
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_real_distribution<float> dist(a, b);
        return dist(rng);
    }

    void setSampleRate(float sr) { sampleRate_ = sr; updatePhaseIncrement(); }
    void updateBpm(int bpm)      { bpm_ = bpm;       updatePhaseIncrement(); }
    void update() {
        if (settings_.reset) {
            state = 0.f;
            justReset_ = true;
            randomValue_ = (settings_.type == LfoType::RandomWalk)
                ? clamp01(randomValue_ + randFloat(-walkSize_, walkSize_))
                : randFloat(-1.f, 1.f);
        }
        updatePhaseIncrement();
    }
    void reset() { justReset_ = true; }

    float process() {
        if (justReset_) {
            justReset_ = false;
            state = 0.f;
            return value = applyDepthAndOffset(0.f);
        }
        float output = 0.f;
        switch (settings_.type) {
            case LfoType::Sin:
                output = (Tables::fastSin(state * Tables::kInv2Pi) + 1.f) * 0.5f;
                break;
            case LfoType::Ramp:
                output = state / (2.f * M_PI);
                break;
            case LfoType::Square:
                output = (state < M_PI) ? 1.f : 0.f;
                break;
            case LfoType::Random:
                if (state + phaseIncrement_ >= 2.f * M_PI)
                    randomValue_ = randFloat(0.f, 1.f);
                output = randomValue_;
                break;
            default: // RandomWalk
                if (state + phaseIncrement_ >= 2.f * M_PI) {
                    float nv = randomValue_ + randFloat(-walkSize_, walkSize_);
                    randomValue_ = (nv > 1.f) ? 2.f - nv : (nv < 0.f ? -nv : nv);
                }
                output = randomValue_;
                break;
        }
        state += phaseIncrement_;
        if (state >= 2.f * M_PI) state -= 2.f * M_PI;
        return value = applyDepthAndOffset(output);
    }
};
