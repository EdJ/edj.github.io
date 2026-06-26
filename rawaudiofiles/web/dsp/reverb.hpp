#pragma once

#include "fastsvf.hpp"
#include "lib/tables.hpp"
#include <algorithm>
#include <memory>

namespace reverbInternal {

class DelayLine {
public:
    DelayLine(size_t size) : bufferSize(size), activeSize(size), pos(0)
        , buffer(std::make_unique<float[]>(size)) { clear(); }
    inline float process(float in) {
        float out = buffer[pos];
        buffer[pos] = in;
        pos = (pos + 1) % activeSize;
        return out;
    }
    inline float tap(size_t d) const { return buffer[(pos + activeSize - d) % activeSize]; }
    void setActiveSize(size_t s) { activeSize = s; pos = pos % activeSize; }
    void clear() { memset(buffer.get(), 0, bufferSize * sizeof(float)); pos = 0; }
private:
    const size_t bufferSize;
    size_t activeSize, pos;
    std::unique_ptr<float[]> buffer;
};

class AllPassFilter {
public:
    AllPassFilter(size_t size) : size(size), pos(0)
        , buffer(std::make_unique<float[]>(size)) { clear(); }
    inline float process(float in, float gain) {
        float d = buffer[pos];
        float s = in + d * gain;
        float out = d - s * gain;
        buffer[pos] = s;
        pos = (pos + 1) % size;
        return out;
    }
    inline float processModulated(float in, float gain, float mod) {
        float d = interpolate(mod);
        float s = in + d * gain;
        float out = d - s * gain;
        buffer[pos] = s;
        pos = (pos + 1) % size;
        return out;
    }
    inline float tap(size_t d) const { return buffer[(pos + size - d) % size]; }
    void clear() { memset(buffer.get(), 0, size * sizeof(float)); pos = 0; }
private:
    inline float interpolate(float offset) const {
        float sizeF = static_cast<float>(size);
        float fi = static_cast<float>(pos) - offset;
        if (fi < 0.f) fi += sizeF; else if (fi >= sizeF) fi -= sizeF;
        size_t i1 = static_cast<size_t>(fi);
        size_t i2 = (i1 + 1) % size;
        float frac = fi - static_cast<float>(i1);
        return buffer[i1] * (1.f - frac) + buffer[i2] * frac;
    }
    const size_t size;
    size_t pos;
    std::unique_ptr<float[]> buffer;
};

class OnePoleFilter {
public:
    void setFrequency(float freq) { a = freq; b = 1.f - freq; }
    inline float process(float in) { return lastOutput = in * a + lastOutput * b; }
    void reset() { lastOutput = 0.f; }
private:
    float a = 0.5f, b = 0.5f, lastOutput = 0.f;
};

class DCBlocker {
public:
    float process(float in) {
        float out = in - lastInput + 0.99f * lastOutput;
        lastInput = in; lastOutput = out;
        return out;
    }
    void reset() { lastInput = lastOutput = 0.f; }
private:
    float lastInput = 0.f, lastOutput = 0.f;
};

class SineLfo {
public:
    void setFrequency(float freq, float sr) { phaseIncrement = (freq * TableSizeF) / sr; }
    void reset() { phase = 0.f; }
    inline float process() {
        size_t i1 = static_cast<size_t>(phase) & (TableSize - 1);
        size_t i2 = (i1 + 1) & (TableSize - 1);
        float frac = phase - static_cast<float>(i1);
        float v = Tables::sineTable[i1] + frac * (Tables::sineTable[i2] - Tables::sineTable[i1]);
        phase += phaseIncrement;
        if (phase >= TableSizeF) phase -= TableSizeF;
        return v;
    }
    inline float processBlock(size_t frames) {
        size_t i1 = static_cast<size_t>(phase) & (TableSize - 1);
        size_t i2 = (i1 + 1) & (TableSize - 1);
        float frac = phase - static_cast<float>(i1);
        float v = Tables::sineTable[i1] + frac * (Tables::sineTable[i2] - Tables::sineTable[i1]);
        phase += phaseIncrement * static_cast<float>(frames);
        if (phase >= TableSizeF) phase -= TableSizeF;
        return v;
    }
private:
    static constexpr size_t TableSize  = 2048;
    static constexpr float  TableSizeF = static_cast<float>(TableSize);
    float phase = 0.f, phaseIncrement = 0.f;
};

} // namespace reverbInternal


// Dattorro plate reverb
// https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf
class PlateReverb {
private:
    static constexpr float DattorroSR     = 29761.0f;
    static constexpr float PreDelayMaxSec = 0.1f;
    static constexpr float InputApf1  = 142.f, InputApf2  = 107.f;
    static constexpr float InputApf3  = 379.f, InputApf4  = 277.f;
    static constexpr float LeftApf1   = 672.f, LeftDelay1 = 4453.f;
    static constexpr float LeftApf2   = 2000.f, LeftDelay2= 3720.f;
    static constexpr float RightApf1  = 908.f, RightDelay1= 4217.f;
    static constexpr float RightApf2  = 2656.f,RightDelay2= 3163.f;
    static constexpr float LeftDelay1Tap1  = 266.f,  LeftDelay1Tap2 = 2974.f;
    static constexpr float LeftApf2Tap1    = 1913.f, LeftDelay2Tap  = 1996.f;
    static constexpr float RightDelay1Tap  = 1990.f, RightApf2Tap   = 187.f;
    static constexpr float RightDelay2Tap  = 1066.f;
    static constexpr float DecayDiffusion1 = 0.70f, DecayDiffusion2 = 0.50f;

    const float sampleRate, srRatio;
    reverbInternal::DelayLine    preDelay;
    reverbInternal::AllPassFilter inputApf1, inputApf2, inputApf3, inputApf4;
    reverbInternal::AllPassFilter leftApf1, leftApf2, rightApf1, rightApf2;
    reverbInternal::DelayLine    leftDelay1, leftDelay2, rightDelay1, rightDelay2;

    float inputGain = 1.f, outputGain = 1.f, decay = 0.85f;
    float diffusionPreDelay = 0.1f, diffusionAllPasses = 0.4f, modDepth = 0.f;
    float leftSum = 0.f, rightSum = 0.f;

    const size_t leftDelay1Tap1_, leftDelay1Tap2_, leftApf2Tap1_;
    const size_t leftDelay2Tap_, rightDelay1Tap_, rightApf2Tap_, rightDelay2Tap_;

    FastSvf inputLowPass, inputHighPass, tankLowPass1, tankHighPass1, tankLowPass2, tankHighPass2;
    reverbInternal::OnePoleFilter leftTankFilter, rightTankFilter;
    reverbInternal::DCBlocker dcBlockL, dcBlockR, dcBlockInputL, dcBlockInputR;
    reverbInternal::SineLfo lfoP1, lfoP2, lfoP3, lfoP4;

public:
    PlateReverb(float sr)
        : sampleRate(sr), srRatio(sr / DattorroSR)
        , preDelay(static_cast<size_t>(sr * PreDelayMaxSec))
        , inputApf1(static_cast<size_t>(InputApf1 * srRatio))
        , inputApf2(static_cast<size_t>(InputApf2 * srRatio))
        , inputApf3(static_cast<size_t>(InputApf3 * srRatio))
        , inputApf4(static_cast<size_t>(InputApf4 * srRatio))
        , leftApf1(static_cast<size_t>(LeftApf1 * srRatio))
        , leftApf2(static_cast<size_t>(LeftApf2 * srRatio))
        , rightApf1(static_cast<size_t>(RightApf1 * srRatio))
        , rightApf2(static_cast<size_t>(RightApf2 * srRatio))
        , leftDelay1(static_cast<size_t>(LeftDelay1 * srRatio))
        , leftDelay2(static_cast<size_t>(LeftDelay2 * srRatio))
        , rightDelay1(static_cast<size_t>(RightDelay1 * srRatio))
        , rightDelay2(static_cast<size_t>(RightDelay2 * srRatio))
        , leftDelay1Tap1_(static_cast<size_t>(LeftDelay1Tap1 * srRatio))
        , leftDelay1Tap2_(static_cast<size_t>(LeftDelay1Tap2 * srRatio))
        , leftApf2Tap1_(static_cast<size_t>(LeftApf2Tap1 * srRatio))
        , leftDelay2Tap_(static_cast<size_t>(LeftDelay2Tap * srRatio))
        , rightDelay1Tap_(static_cast<size_t>(RightDelay1Tap * srRatio))
        , rightApf2Tap_(static_cast<size_t>(RightApf2Tap * srRatio))
        , rightDelay2Tap_(static_cast<size_t>(RightDelay2Tap * srRatio))
    {
        inputLowPass.Init(); inputHighPass.Init();
        tankLowPass1.Init(); tankHighPass1.Init();
        tankLowPass2.Init(); tankHighPass2.Init();
        setDefaultParameters();
        clear();
    }

    void process(float* bufL, float* bufR, size_t frames) {
        const float mod1 = lfoP1.processBlock(frames) * modDepth;
        const float mod2 = lfoP2.processBlock(frames) * modDepth;
        const float mod3 = lfoP3.processBlock(frames) * modDepth;
        const float mod4 = lfoP4.processBlock(frames) * modDepth;

        for (size_t i = 0; i < frames; ++i) {
            float diffused = (dcBlockInputL.process(bufL[i]) + dcBlockInputR.process(bufR[i])) * inputGain;
            diffused = inputHighPass.process_high(diffused);
            diffused = inputLowPass.process_low(diffused);
            diffused = preDelay.process(diffused);
            diffused = inputApf1.process(diffused, 0.75f);
            diffused = inputApf2.process(diffused, 0.75f);
            diffused = inputApf3.process(diffused, 0.625f);
            diffused = inputApf4.process(diffused, 0.625f);

            leftSum  += diffused * diffusionAllPasses;
            rightSum += diffused * diffusionAllPasses;
            leftSum  = fast_tanh(leftSum);
            rightSum = fast_tanh(rightSum);

            float ld2 = leftSum;
            ld2 = leftApf1.processModulated(ld2, DecayDiffusion1, mod1);
            ld2 = leftDelay1.process(ld2);
            ld2 = leftTankFilter.process(ld2);
            ld2 = tankLowPass1.process_low(ld2);
            ld2 *= decay;
            ld2 = leftApf2.processModulated(ld2, DecayDiffusion2, mod2);
            ld2 = leftDelay2.process(ld2);

            float rd2 = rightSum;
            rd2 = rightApf1.processModulated(rd2, DecayDiffusion1, mod3);
            rd2 = rightDelay1.process(rd2);
            rd2 = rightTankFilter.process(rd2);
            rd2 = tankLowPass2.process_low(rd2);
            rd2 *= decay;
            rd2 = rightApf2.processModulated(rd2, DecayDiffusion2, mod4);
            rd2 = rightDelay2.process(rd2);

            rightSum = ld2;
            leftSum  = rd2;

            float outL = leftDelay1.tap(leftDelay1Tap1_);
            outL += leftDelay1.tap(leftDelay1Tap2_) * 0.25f;
            outL -= leftApf2.tap(leftApf2Tap1_)    * 0.125f;
            outL += leftDelay2.tap(leftDelay2Tap_)  * 0.125f;
            outL -= rightDelay1.tap(rightDelay1Tap_)* 0.0625f;
            outL -= rightApf2.tap(rightApf2Tap_)    * 0.0625f;
            outL -= rightDelay2.tap(rightDelay2Tap_)* 0.03125f;
            bufL[i] = dcBlockL.process(outL * outputGain);

            float outR = rightDelay1.tap(leftDelay1Tap1_);
            outR += rightDelay1.tap(leftDelay1Tap2_) * 0.25f;
            outR -= rightApf2.tap(leftApf2Tap1_)    * 0.125f;
            outR += rightDelay2.tap(leftDelay2Tap_)  * 0.125f;
            outR -= leftDelay1.tap(rightDelay1Tap_)  * 0.0625f;
            outR -= leftApf2.tap(rightApf2Tap_)      * 0.0625f;
            outR -= leftDelay2.tap(rightDelay2Tap_)  * 0.03125f;
            bufR[i] = dcBlockR.process(outR * outputGain);
        }
    }

    void clear() {
        preDelay.clear(); inputApf1.clear(); inputApf2.clear(); inputApf3.clear(); inputApf4.clear();
        leftApf1.clear(); leftDelay1.clear(); leftApf2.clear(); leftDelay2.clear();
        rightApf1.clear(); rightDelay1.clear(); rightApf2.clear(); rightDelay2.clear();
        leftSum = rightSum = 0.f;
        lfoP1.reset(); lfoP2.reset(); lfoP3.reset(); lfoP4.reset();
        dcBlockL.reset(); dcBlockR.reset(); dcBlockInputL.reset(); dcBlockInputR.reset();
        leftTankFilter.reset(); rightTankFilter.reset();
    }

    void setDefaultParameters() {
        inputGain = outputGain = 1.f;
        decay    = 0.85f;
        modDepth = 0.5f * 4.f * srRatio;
        lfoP1.setFrequency(0.10f, sampleRate);
        lfoP2.setFrequency(0.15f, sampleRate);
        lfoP3.setFrequency(0.12f, sampleRate);
        lfoP4.setFrequency(0.18f, sampleRate);
        inputLowPass.set_f_q(0.24487524f,   0.1f);
        inputHighPass.set_f_q(0.000416666f, 0.1f);
        tankLowPass1.set_f_q(0.24487524f,   0.1f);
        tankHighPass1.set_f_q(0.0025f,      0.1f);
        tankLowPass2.set_f_q(0.24487524f,   0.1f);
        tankHighPass2.set_f_q(0.0025f,      0.1f);
        leftTankFilter.setFrequency(0.2f);
        rightTankFilter.setFrequency(0.2f);
    }

    void setDecay(float v)     { decay      = std::clamp(v, 0.1f, 0.9998f); }
    void setModDepth(float v)  { modDepth   = std::clamp(v, 0.05f, 0.5f) * 4.f * srRatio; }
    void setOutputGain(float v){ outputGain = std::clamp(v, 0.1f, 1.f); }
    void setLowPass(float f, float q = 0.1f)  { inputLowPass.set_f_q(f, q); tankLowPass1.set_f_q(f, q); tankLowPass2.set_f_q(f, q); }
    void setHighPass(float f, float q = 0.1f) { inputHighPass.set_f_q(f, q); }
    void setPreDelay(float v)  { preDelay.setActiveSize(static_cast<size_t>(v * sampleRate * 0.1f)); }
    void setDiffusion(float v) { diffusionAllPasses = v; diffusionPreDelay = 1.f - v; }

    static inline float fast_tanh(float x) {
        if (x >  3.f) return  1.f;
        if (x < -3.f) return -1.f;
        float x2 = x * x;
        return x * (27.f + x2) / (27.f + 9.f * x2);
    }
};
