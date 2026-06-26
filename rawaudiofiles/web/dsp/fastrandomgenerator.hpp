#pragma once

#include <cstdint>
#include <ctime>
#include "vdsp.hpp"

class FastRandomGenerator {
private:
    uint64_t state;
    float multiplier;
    float offset;
    static constexpr float normalizedMult = 1.0f / static_cast<float>(UINT64_MAX);

public:
    FastRandomGenerator()
        : state(static_cast<uint64_t>(time(nullptr)))
        , multiplier(2.0f / static_cast<float>(UINT64_MAX))
        , offset(-1.0f) {}

    FastRandomGenerator(float mult, float off)
        : state(static_cast<uint64_t>(time(nullptr)))
        , multiplier(mult / static_cast<float>(UINT64_MAX))
        , offset(off) {}

    void update(float newMultiplier, float newOffset) {
        multiplier = newMultiplier / static_cast<float>(UINT64_MAX);
        offset = newOffset;
    }

    inline float doShift() {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return static_cast<float>(state);
    }

    int nextInRange(int start, int end) {
        float r = doShift() * normalizedMult;
        return static_cast<int>(r * static_cast<float>(end - start)) + start;
    }

    float nextFloat() { return doShift() * multiplier + offset; }

    void fill(float* buffer, int size) {
        uint64_t localState = state;
        for (int i = 0; i < size; i++) {
            localState ^= localState << 13;
            localState ^= localState >> 7;
            localState ^= localState << 17;
            buffer[i] = static_cast<float>(localState);
        }
        state = localState;
        vDSP_vsmul(buffer, 1, &multiplier, buffer, 1, size);
        vDSP_vsadd(buffer, 1, &offset,     buffer, 1, size);
    }
};
