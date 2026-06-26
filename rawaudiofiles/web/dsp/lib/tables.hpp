#ifndef TABLES
#define TABLES

#include <cmath>
#include <array>
#include <algorithm>
#include "../vdsp.hpp"

class MidiTables {
    static constexpr int TABLE_SIZE = 128;
    template<typename Func>
    static std::array<float, TABLE_SIZE> makeTable(Func f) {
        std::array<float, TABLE_SIZE> arr = {};
        for (int i = 0; i < TABLE_SIZE; ++i) arr[i] = f(i);
        return arr;
    }
public:
    static inline const auto midiToPitchFactor = makeTable([](int i) {
        float semitoneFactor = std::pow(2.0f, 1.0f / 12.0f);
        return std::pow(semitoneFactor, static_cast<float>(i) - 60.0f);
    });
    static inline const auto midiToHz = makeTable([](int i) {
        float noteOffset = (static_cast<float>(i) - 69.0f) / 12.0f;
        return 440.0f * std::pow(2.0f, noteOffset);
    });
    template<const std::array<float, TABLE_SIZE>& table>
    static constexpr inline float getValue(float x) noexcept {
        int idx = static_cast<int>(x * float(TABLE_SIZE - 1));
        return table[idx];
    }
    static constexpr inline float getHz(int x) noexcept { return midiToHz[x]; }
    static constexpr inline float getPitchFactor(float x) noexcept { return midiToPitchFactor[x]; }
};

class Tables {
public:
    static constexpr int TABLE_SIZE = 2048;
    static constexpr float SampleRateF = 48000.0f;

private:
    template<typename Func>
    static std::array<float, TABLE_SIZE> makeTable(Func f) {
        std::array<float, TABLE_SIZE> arr = {};
        for (int i = 0; i < TABLE_SIZE; ++i) arr[i] = f(i);
        return arr;
    }

public:
    static inline const auto exponentialTable = makeTable([](int i) {
        constexpr float C = 3.0f;
        return std::pow(float(i), C) / std::pow(float(TABLE_SIZE), C);
    });
    static inline const auto exponentialTableSlow = makeTable([](int i) {
        constexpr float C = 2.0f;
        return std::pow(float(i), C) / std::pow(float(TABLE_SIZE), C);
    });
    static inline const auto exponentialMidpoint = makeTable([](int i) {
        constexpr float C = 2.0f;
        constexpr int half = TABLE_SIZE / 2;
        if (i < half)
            return std::pow(float(i), C) / std::pow(float(TABLE_SIZE), C);
        float n = static_cast<float>(i - half) / static_cast<float>(half);
        return 1.0f + (1.0f * std::pow(n, C));
    });
    static inline const auto stmResonanceNormalized = makeTable([](int i) {
        constexpr float minQ = 0.5f, maxQ = 10.0f;
        float n = float(i) / float(TABLE_SIZE - 1);
        return minQ * std::pow(maxQ / minQ, n);
    });
    static inline const auto filterNormalized = makeTable([](int i) {
        constexpr float minF = 20.0f, maxF = 24000.0f;
        float n = float(i) / float(TABLE_SIZE - 1);
        return (minF * std::pow(maxF / minF, n)) / SampleRateF;
    });
    static inline const auto reverbTail = makeTable([](int i) {
        float n = float(i) / float(TABLE_SIZE - 1);
        return 0.9f + std::pow(n, 6.0f) * (0.9999f - 0.9f);
    });
    static inline const auto sineTable = makeTable([](int i) {
        return std::sin(2.0f * float(M_PI) * float(i) / float(TABLE_SIZE));
    });

    static constexpr float kInv2Pi = 1.0f / (2.0f * float(M_PI));

    static inline float fastSin(float phase01) noexcept {
        float idx = phase01 * float(TABLE_SIZE);
        int i0 = static_cast<int>(idx) & (TABLE_SIZE - 1);
        int i1 = (i0 + 1) & (TABLE_SIZE - 1);
        float frac = idx - static_cast<float>(i0);
        return sineTable[i0] + frac * (sineTable[i1] - sineTable[i0]);
    }

    template<const std::array<float, TABLE_SIZE>& table>
    static constexpr inline float getValue(float x) noexcept {
        int idx = static_cast<int>(x * float(TABLE_SIZE - 1));
        return table[std::clamp(idx, 0, TABLE_SIZE - 1)];
    }

    static inline void process(const float* table, float* buffer, int size, int tableSize = TABLE_SIZE) noexcept {
        float scale = float(tableSize - 2);
        vDSP_vsmul(buffer, 1, &scale, buffer, 1, size);
        vDSP_vlint(table, buffer, 1, buffer, 1, size, tableSize);
    }
};

#endif // TABLES
