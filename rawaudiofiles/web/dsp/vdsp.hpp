#pragma once

// vDSP shim for WebAssembly — drop-in replacement for Apple Accelerate vDSP calls.
// Adds vvexpf on top of the original rawaudiofiles/vdsp.hpp set.

#include <cstring>
#include <algorithm>
#include <cmath>

#define vDSP_vgen(A, B, C, IC, N)                    \
    do {                                               \
        int n_ = (N);                                  \
        if (n_ <= 0) break;                            \
        float a_ = *(A); float b_ = *(B);             \
        if (n_ == 1) { (C)[0] = b_; }                 \
        else {                                         \
            float step_ = (b_ - a_) / (float)(n_ - 1);\
            for (int i = 0; i < n_; i++)              \
                (C)[i] = a_ + step_ * (float)i;       \
        }                                              \
    } while (0)

#define vDSP_vramp(A, B, C, IC, N)                    \
    do {                                               \
        int n_ = (N);                                  \
        float start_ = *(A); float delta_ = *(B);     \
        for (int i = 0; i < n_; i++)                  \
            (C)[i] = start_ + delta_ * (float)i;      \
    } while (0)

#define vDSP_vsmul(A, IA, B, C, IC, N)               \
    do {                                               \
        int n_ = (N); float s_ = *(B);                \
        for (int i = 0; i < n_; i++)                  \
            (C)[i] = (A)[i] * s_;                     \
    } while (0)

#define vDSP_vmul(A, IA, B, IB, C, IC, N)            \
    do {                                               \
        int n_ = (N);                                  \
        for (int i = 0; i < n_; i++)                  \
            (C)[i] = (A)[i] * (B)[i];                 \
    } while (0)

#define vDSP_vadd(A, IA, B, IB, C, IC, N)            \
    do {                                               \
        int n_ = (N);                                  \
        for (int i = 0; i < n_; i++)                  \
            (C)[i] = (A)[i] + (B)[i];                 \
    } while (0)

#define vDSP_vsub(B, IB, A, IA, C, IC, N)            \
    do {                                               \
        int n_ = (N);                                  \
        for (int i = 0; i < n_; i++)                  \
            (C)[i] = (A)[i] - (B)[i];                 \
    } while (0)

#define vDSP_vsadd(A, IA, B, C, IC, N)               \
    do {                                               \
        int n_ = (N); float s_ = *(B);                \
        for (int i = 0; i < n_; i++)                  \
            (C)[i] = (A)[i] + s_;                     \
    } while (0)

#define vDSP_vma(A, IA, B, IB, C, IC, D, ID, N)      \
    do {                                               \
        int n_ = (N);                                  \
        for (int i = 0; i < n_; i++)                  \
            (D)[i] = (A)[i] * (B)[i] + (C)[i];       \
    } while (0)

#define vDSP_vclr(C, IC, N)                           \
    do {                                               \
        if ((N) > 0) memset((C), 0, sizeof(float) * (N)); \
    } while (0)

#define vDSP_vfill(A, C, IC, N)                       \
    do {                                               \
        int n_ = (N); float a_ = *(A);                \
        for (int i = 0; i < n_; i++) (C)[i] = a_;    \
    } while (0)

#define vDSP_vclip(A, IA, B, C, D, ID, N)            \
    do {                                               \
        int n_ = (N);                                  \
        float lo_ = *(B); float hi_ = *(C);           \
        for (int i = 0; i < n_; i++)                  \
            (D)[i] = std::clamp((A)[i], lo_, hi_);    \
    } while (0)

#define vDSP_vindex(TABLE, A, IA, B, IC, N)           \
    do {                                               \
        int n_ = (N);                                  \
        for (int i = 0; i < n_; i++)                  \
            (B)[i] = (TABLE)[static_cast<int>((A)[i])]; \
    } while (0)

#define vDSP_vlint(TABLE, A, IA, B, IC, N, TABLESIZE) \
    do {                                               \
        int n_ = (N);                                  \
        for (int i = 0; i < n_; i++) {                \
            int idx_ = static_cast<int>((A)[i]);       \
            if (idx_ >= (TABLESIZE) - 1) {             \
                (B)[i] = (TABLE)[(TABLESIZE) - 1];    \
            } else {                                   \
                float f_ = (A)[i] - (float)idx_;      \
                (B)[i] = (TABLE)[idx_] + f_ * ((TABLE)[idx_+1] - (TABLE)[idx_]); \
            }                                          \
        }                                              \
    } while (0)

#define vDSP_vfrac(A, IA, B, IC, N)                   \
    do {                                               \
        int n_ = (N);                                  \
        for (int i = 0; i < n_; i++) {                \
            int idx_ = static_cast<int>((A)[i]);      \
            (B)[i] = (A)[i] - (float)idx_;            \
        }                                              \
    } while (0)

#define vDSP_maxmgv(A, IA, C, N)                      \
    do {                                               \
        int n_ = (N); float m_ = 0.f;                 \
        for (int i = 0; i < n_; i++) {                \
            float v_ = std::fabs((A)[i]);             \
            if (v_ > m_) m_ = v_;                     \
        }                                              \
        *(C) = m_;                                     \
    } while (0)

// Vectorised transcendentals (N is a pointer, matching vecLib signature)
#define vvsinf(A, B, N)                               \
    do {                                               \
        int n_ = *(N);                                 \
        for (int i = 0; i < n_; i++)                  \
            (A)[i] = std::sin((B)[i]);                \
    } while (0)

#define vvexpf(A, B, N)                               \
    do {                                               \
        int n_ = *(N);                                 \
        for (int i = 0; i < n_; i++)                  \
            (A)[i] = std::exp((B)[i]);                \
    } while (0)

#define vvtanhf(A, B, N)                              \
    do {                                               \
        int n_ = *(N);                                 \
        for (int i = 0; i < n_; i++)                  \
            (A)[i] = std::tanh((B)[i]);               \
    } while (0)

#define vvpowf(A, B, C, N)                            \
    do {                                               \
        int n_ = *(N);                                 \
        for (int i = 0; i < n_; i++)                  \
            (A)[i] = std::pow(*(B), (C)[i]);          \
    } while (0)
