// FastSvf — portable C++, ARM NEON variants stripped (not needed for WASM)
// Source: Easter/DSP/custom/fastsvf.hpp

#ifndef FASTSVF
#define FASTSVF

#include <math.h>

class FastSvf {
public:
    FastSvf() {}

    void Init() { set_f_q(0.01f, 1.0f); Reset(); }
    void Reset() { state_1_ = state_2_ = 0.0f; }

    inline float fast_tan(float f) {
        static constexpr float pi  = float(M_PI);
        static constexpr float pi3 = pi * pi * pi;
        static constexpr float pi5 = pi3 * pi * pi;
        static constexpr float a   = 3.260e-01f * pi3;
        static constexpr float b   = 1.823e-01f * pi5;
        float f2 = f * f;
        return f * (pi + f2 * (a + b * f2));
    }

    inline void set(const FastSvf& f) { g_ = f.g(); r_ = f.r(); rg_ = f.rg(); h_ = f.h(); }

    inline void set_f_q(float f, float resonance) {
        if (f == lastF_ && resonance == lastResonance_) return;
        lastF_ = f; lastResonance_ = resonance;
        g_  = fast_tan(f);
        r_  = 1.0f / resonance;
        rg_ = r_ + g_;
        h_  = 1.0f / (1.0f + r_ * g_ + g_ * g_);
    }

    inline float process_low(float in) {
        float hp = (in - rg_ * state_1_ - state_2_) * h_;
        float g_hp = g_ * hp;
        float bp = g_hp + state_1_;
        state_1_ += 2.0f * g_hp;
        float g_bp = g_ * bp;
        float lp = g_bp + state_2_;
        state_2_ += 2.0f * g_bp;
        return lp;
    }

    inline float process_high(float in) {
        float hp = (in - rg_ * state_1_ - state_2_) * h_;
        float g_hp = g_ * hp;
        float bp = g_hp + state_1_;
        state_1_ += 2.0f * g_hp;
        state_2_ += 2.0f * (g_ * bp);
        return hp;
    }

    inline float process_band(float in) {
        float hp = (in - rg_ * state_1_ - state_2_) * h_;
        float g_hp = g_ * hp;
        float bp = g_hp + state_1_;
        state_1_ += 2.0f * g_hp;
        state_2_ += 2.0f * (g_ * bp);
        return bp;
    }

    inline void process_low(float* __restrict__ in, size_t size) {
        float s1 = state_1_, s2 = state_2_;
        while (size--) {
            float hp = (*in - rg_ * s1 - s2) * h_;
            float g_hp = g_ * hp;
            float bp = g_hp + s1;
            s1 += 2.0f * g_hp;
            float g_bp = g_ * bp;
            *in++ = g_bp + s2;
            s2 += 2.0f * g_bp;
        }
        state_1_ = s1; state_2_ = s2;
    }

    inline void process_high(float* __restrict__ in, size_t size) {
        float s1 = state_1_, s2 = state_2_;
        while (size--) {
            float hp = (*in - rg_ * s1 - s2) * h_;
            float g_hp = g_ * hp;
            float bp = g_hp + s1;
            s1 += 2.0f * g_hp;
            s2 += 2.0f * (g_ * bp);
            *in++ = hp;
        }
        state_1_ = s1; state_2_ = s2;
    }

    inline void process_band(float* __restrict__ in, size_t size) {
        float s1 = state_1_, s2 = state_2_;
        while (size--) {
            float hp = (*in - rg_ * s1 - s2) * h_;
            float g_hp = g_ * hp;
            float bp = g_hp + s1;
            s1 += 2.0f * g_hp;
            s2 += 2.0f * (g_ * bp);
            *in++ = bp;
        }
        state_1_ = s1; state_2_ = s2;
    }

    inline float g()  const { return g_; }
    inline float r()  const { return r_; }
    inline float rg() const { return rg_; }
    inline float h()  const { return h_; }

private:
    float g_, r_, rg_, h_;
    float lastF_ = -1.0f, lastResonance_ = -1.0f;
    float state_1_ = 0.f, state_2_ = 0.f;
};

#endif // FASTSVF
