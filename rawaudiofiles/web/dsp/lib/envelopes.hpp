#ifndef ENVELOPES
#define ENVELOPES

#include <algorithm>
#include "tables.hpp"

enum class EnvType { Amp, Mod };

template <EnvType Type, int MaxBufferCapacity>
struct DecayEnv {
    float buffer[MaxBufferCapacity] = {};
    float state = 0.0f;
    float delta = 0.0f;
    bool isSlow = false;

    static constexpr float zero = 0.0f;
    static constexpr float one  = 1.0f;
    float samplesPerMs;

    inline void setDecay(int ms) {
        delta = -(1.0f / (static_cast<float>(ms) * samplesPerMs));
    }
    inline void setSamples(int samples) {
        delta = -(1.0f / static_cast<float>(samples));
    }

    inline void process(bool trigger, int size) {
        if constexpr (Type == EnvType::Amp) {
            if (trigger) {
                state = std::max(state, 0.0f);
                vDSP_vgen(&state, &one, buffer, 1, size);
                state = one;
                return;
            }
        } else {
            if (trigger) state = one;
        }
        vDSP_vramp(&state, &delta, buffer, 1, size);
        vDSP_vclip(buffer, 1, &zero, &one, buffer, 1, size);
        state = std::clamp(buffer[size - 1] + delta, 0.0f, 1.0f);
        if constexpr (Type == EnvType::Amp) {
            const float* table = isSlow ? Tables::exponentialTableSlow.data()
                                        : Tables::exponentialTable.data();
            Tables::process(table, buffer, size);
        }
    }
};

enum class AttackDecayEnvState { Attack, RampToAttack, Decay, Done };

template <EnvType Type, int MaxBufferCapacity>
struct AttackDecayEnv {
    float buffer[MaxBufferCapacity] = {};
    float state = 0.0f;
    float delta = 0.0f;
    float deltaDecay = 0.0f;
    float deltaAttack = 0.0f;
    float floor = 0.0f;
    bool isSlow = false;
    AttackDecayEnvState mode = AttackDecayEnvState::Attack;

    static constexpr float zero = 0.0f;
    static constexpr float one  = 1.0f;
    static constexpr float maximumEnvelopeRamp = 72.0f / 48000.0f;
    float samplesPerMs;

    inline void update(int attackMs, int decayMs) {
        deltaAttack = 1.0f / (static_cast<float>(attackMs) * samplesPerMs);
        deltaDecay  = -(1.0f / (static_cast<float>(decayMs) * samplesPerMs));
    }

    inline void process(bool trigger, int size) {
        if constexpr (Type == EnvType::Amp) {
            if (trigger) {
                state = std::max(state, 0.0f);
                floor = state;
                mode  = AttackDecayEnvState::RampToAttack;
                delta = deltaAttack;
                return;
            }
        } else {
            if (trigger) {
                state = zero;
                mode  = AttackDecayEnvState::Attack;
                delta = deltaAttack;
            }
        }

        int remaining = size, block = size, offset = 0;
        float target = 0.0f;
        while (remaining > 0) {
            block  = remaining;
            offset = size - remaining;
            switch (mode) {
                case AttackDecayEnvState::Done:
                    vDSP_vclr(buffer + offset, 1, block);
                    remaining -= block;
                    break;
                case AttackDecayEnvState::RampToAttack: {
                    floor  += deltaAttack * block;
                    target  = state - maximumEnvelopeRamp * block;
                    if (target <= floor) {
                        target = floor;
                        block  = std::max(0, static_cast<int>((state - floor) / maximumEnvelopeRamp));
                        mode   = AttackDecayEnvState::Attack;
                        delta  = deltaAttack;
                        vDSP_vgen(&state, &target, buffer + offset, 1, block);
                        state  = target + delta;
                        remaining -= block;
                        break;
                    }
                    vDSP_vgen(&state, &target, buffer + offset, 1, block);
                    state  = target + maximumEnvelopeRamp;
                    remaining -= block;
                    break;
                }
                case AttackDecayEnvState::Attack: {
                    target = state + delta * block;
                    if (target >= 1.0f) {
                        target = 1.0f;
                        block  = std::max(0, static_cast<int>((1.0f - state) / delta));
                        mode   = AttackDecayEnvState::Decay;
                        delta  = deltaDecay;
                    }
                    vDSP_vgen(&state, &target, buffer + offset, 1, block);
                    state  = target + delta;
                    remaining -= block;
                    break;
                }
                case AttackDecayEnvState::Decay: {
                    target = state + delta * block;
                    if (target <= 0.0f) {
                        target = 0.0f;
                        block  = std::max(0, static_cast<int>(-state / delta));
                        mode   = AttackDecayEnvState::Done;
                        delta  = deltaAttack;
                    }
                    vDSP_vgen(&state, &target, buffer + offset, 1, block);
                    state  = target + delta;
                    remaining -= block;
                    break;
                }
            }
        }
        if constexpr (Type == EnvType::Amp) {
            const float* table = isSlow ? Tables::exponentialTableSlow.data()
                                        : Tables::exponentialTable.data();
            Tables::process(table, buffer, size);
        }
    }
};

#endif // ENVELOPES
