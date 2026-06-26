//
//  overdrive.hpp
//  Jolt
//
//  Created by Ed James on 30/04/2025.
//

#ifndef OVERDRIVE
#define OVERDRIVE

#include <cmath>

// Oversample - TODO
// use vdsp_vadd to add to a zeroed buffer with a stride of oversample rate (e.g. 2)
// Filter with 0.45 Nyquist
// process
// Filter with 0.45 Nyquist
// use vindex to take every x samples (or again, vadd it with a stride, check performance)
// Potentially divide by oversampling rate (but this seems odd?)
// https://www.dafx12.york.ac.uk/papers/dafx12_submission_45.pdf - Look more into various algorithms

class QuickOverdrive {
public:
    float amount;

    QuickOverdrive(float amt = 0.0f)
        : amount(amt) { update(amt); }

    void update(float newAmount) {
        amount = newAmount;
        const float gain = 1.0f + newAmount;
        const float a = powf(10.0f, newAmount * 65.0f * 0.05f);
        gainFactor = powf(10.0f, gain * 0.025f) * (1.0f / sqrtf(a + 1.0f));
        aMinus1 = a - 1.0f;
        aCache = a;
    }

    void processBuffer(float* buffer, size_t size) const {
        for (size_t i = 0; i < size; ++i) {
            float x = buffer[i];
            float absX = fabsf(x);
            float denominator = x * x + aMinus1 * absX + 1.0f;
            buffer[i] = gainFactor * (x * (absX + aCache) / denominator);
        }
    }
    
    // Potentially faster? Profile at some point
    void processBufferAlt(float* buffer, size_t size) const {
        // Precalculate all constants outside the loop
        const float gain = 1.0f + amount;
        
        // Fast approximations for expensive math operations
        // Using bit manipulation for faster pow approximation (exp2(log2(x)*y))
        const float expTerm = amount * 65.0f * 0.05f * 3.32192809489f; // 3.32... is log2(10)
        const float a = (1 << (int)(expTerm)) * (1.0f + (expTerm - (int)(expTerm)));
        
        const float gainExp = gain * 0.05f * 3.32192809489f;
        const float gainTerm = (1 << (int)(gainExp)) * (1.0f + (gainExp - (int)(gainExp)));
        
        // Fast inverse square root approximation
        const float invSqrt = fastInvSqrt(a + 1.0f);
        const float gainFactor = gainTerm * invSqrt;
        const float aMinus1 = a - 1.0f;
        
        // Simple, straightforward loop with optimized math functions
        for (size_t i = 0; i < size; ++i) {
            float x = buffer[i];
            float absX = fabsf(x); // Use faster C function instead of std::fabs
            float denominator = x * x + aMinus1 * absX + 1.0f;
            buffer[i] = gainFactor * (x * (absX + a) / denominator);
        }
    }

private:
    float gainFactor = 0.0f;
    float aMinus1 = 0.0f;
    float aCache = 0.0f;

public:
    // Fast inverse square root approximation (famous "Quake III" algorithm)
    inline float fastInvSqrt(float x) const {
        const float xhalf = 0.5f * x;
        int i = *(int*)&x;        // Reinterpret as integer
        i = 0x5f3759df - (i >> 1); // Magic number for approximation
        x = *(float*)&i;           // Reinterpret as float
        x = x * (1.5f - xhalf * x * x); // Newton iteration for accuracy
        return x;
    }
};

#endif // OVERDRIVE
