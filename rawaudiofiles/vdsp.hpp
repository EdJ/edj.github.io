//
//  vdsp.hpp
//  Jolt
//
//  Created by Ed James on 13/09/2025.
//  Absolute shenanigans
//

#pragma once

// vDSP_vgen(A, B, C, IC, N)
// Linear interpolation A → B
#define vDSP_vgen(A, B, C, IC, N)                    \
    do {                                                 \
        int n_ = (N);                            \
        if (n_ <= 0) break;                              \
        float a = *(A);                                  \
        float b = *(B);                                  \
        if (n_ == 1) {                                   \
            (C)[0] = b;                                  \
        } else {                                         \
            float step = (b - a) / (float)(n_ - 1);      \
            for (int i = 0; i < n_; i++)         \
                (C)[i] = a + step * (float)i;            \
        }                                                \
    } while (0)

// vDSP_vramp(A, B, C, IC, N)
// Ramp: start + delta*i
#define vDSP_vramp(A, B, C, IC, N)                       \
    do {                                                 \
        int n_ = (N);                            \
        if (n_ <= 0) break;                              \
        float start = *(A);                              \
        float delta = *(B);                              \
        for (int i = 0; i < n_; i++)             \
            (C)[i] = start + delta * (float)i;           \
    } while (0)

// vDSP_vsmul(A, IA, B, C, IC, N)
#define vDSP_vsmul(A, IA, B, C, IC, N)                       \
    do {                                                 \
        int n_ = (N);                            \
        for (int i = 0; i < n_; i++)             \
            (C)[i] = (A)[i] * *(B);           \
    } while (0)

// Vector multiply (mostly for envelopes)
#define vDSP_vmul(A, IA, B, IB, C, IC, N)                       \
    do {                                                 \
        int n_ = (N);                            \
        for (int i = 0; i < n_; i++)             \
            (C)[i] = (A)[i] * (B)[i];           \
    } while (0)

// vDSP_vadd(A, IA, B, IB, C, IC, N)
#define vDSP_vadd(A, IA, B, IB, C, IC, N)                       \
    do {                                                 \
        int n_ = (N);                            \
        for (int i = 0; i < n_; i++)             \
            (C)[i] = (A)[i] + (B)[i];           \
    } while (0)

// Vector subtract
#define vDSP_vsub(B, IB, A, IA, C, IC, N)                       \
    do {                                                 \
        int n_ = (N);                            \
        for (int i = 0; i < n_; i++)             \
            (C)[i] = (A)[i] - (B)[i];           \
    } while (0)

// vDSP_vsadd(A, IA, B, C, IC, N)
#define vDSP_vsadd(A, IA, B, C, IC, N)                       \
    do {                                                 \
        int n_ = (N);                            \
        for (int i = 0; i < n_; i++)             \
            (C)[i] = (A)[i] + *(B);           \
    } while (0)

// vDSP_vma(A, IA, B, IB, C, IC, D, ID, N)
#define vDSP_vma(A, IA, B, IB, C, IC, D, ID, N)                       \
    do {                                                 \
        int n_ = (N);                            \
        for (int i = 0; i < n_; i++)             \
            (D)[i] = ((A)[i] * (B)[i]) + (C)[i];           \
    } while (0)

// vDSP_vclr(C, IC, N)
// Zero using memset
#define vDSP_vclr(C, IC, N)                            \
    do {                                                 \
        if ((N) <= 0) break;                             \
        memset((C), 0, sizeof(float) * (N));             \
    } while (0)

// vDSP_vfill(A, C, IC, N)
// Fill buffer
#define vDSP_vfill(A, C, IC, N)                            \
    do {                                                 \
        int n_ = (N);                            \
        if (n_ <= 0) break;                              \
        float a = *(A);                              \
        for (int i = 0; i < n_; i++)             \
            (C)[i] = a;           \
    } while (0)

// Clamp
#define vDSP_vclip(A, IA, B, C, D, ID, N)                            \
    do {                                                 \
        int n_ = (N);                            \
        if (n_ <= 0) break;                              \
        for (int i = 0; i < n_; i++)             \
            (D)[i] = std::clamp((A)[i], *(B), *(C));           \
    } while (0)

// vindex - table lookup without lerp
#define vDSP_vindex(TABLE, A, IA, B, IC, N)                            \
    do {                                                 \
        int n_ = (N);                            \
        if (n_ <= 0) break;                              \
        for (int i = 0; i < n_; i++)   {          \
            int idx = static_cast<int>((A)[i]); \
            (B)[i] = (TABLE)[idx];           \
        } \
    } while (0)

// Vlint - lookup with interpolation
#define vDSP_vlint(TABLE, A, IA, B, IC, N, TABLESIZE)                            \
    do {                                                 \
        int n_ = (N);                            \
        if (n_ <= 0) break;                              \
        for (int i = 0; i < n_; i++)   {          \
            int idx = static_cast<int>((A)[i]); \
            if (idx == TABLESIZE - 1) { \
                (B)[i] = (TABLE)[idx]; \
            } else { \
                float fract = (A)[i] - idx; \
                float val = (TABLE)[idx]; \
                (B)[i] = val + (((TABLE)[idx + 1] - val) * fract);           \
            } \
        } \
    } while (0)

// Vectorized frac
#define vDSP_vfrac(A, IA, B, IC, N)                            \
    do {                                                 \
        int n_ = (N);                            \
        if (n_ <= 0) break;                              \
        for (int i = 0; i < n_; i++)   {          \
            int idx = static_cast<int>((A)[i]); \
            (B)[i] = (A)[i] - idx;           \
        } \
    } while (0)



// Not technically in vdsp:
// Calculate sines of vector
#define vvsinf(A, B, N)                            \
    do {                                                 \
        if ((N) <= 0) break;                             \
        int n_ = *(N);                            \
        for (int i = 0; i < n_; i++)             \
            (A)[i] = std::sin(B[i]);           \
    } while (0)

// Calculate tanhs of vector
#define vvtanhf(A, B, N)                            \
    do {                                                 \
        if ((N) <= 0) break;                             \
        int n_ = *(N);                            \
        for (int i = 0; i < n_; i++)             \
            (A)[i] = std::tanh(B[i]);           \
    } while (0)

// Calculate sines of vector
#define vvpowf(A, B, C, N)                            \
    do {                                                 \
        if ((N) <= 0) break;                             \
        int n_ = *(N);                            \
        for (int i = 0; i < n_; i++)             \
            (A)[i] = std::pow(*(B), C[i]);           \
    } while (0)
