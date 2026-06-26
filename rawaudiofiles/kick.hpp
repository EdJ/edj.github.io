//
//  grainhat.hpp
//  Jolt
//
//  Created by Ed James on 21/10/2025.
//

#ifndef KICK
#define KICK

#import "./fastrandomgenerator.hpp"
#import "./lib/envelopes.hpp"
#import "./lib/tables.hpp"
#import "./fastsvf.hpp"

class MidiToHz {
private:
    static constexpr int TABLE_SIZE = 128;

    static std::array<float, TABLE_SIZE> generateTable() {
        std::array<float, TABLE_SIZE> arr{};
        for (int i = 0; i < TABLE_SIZE; ++i) {
            float noteOffset = static_cast<float>(i - 69) / 12.0f;
            arr[i] = 440.0f * powf(2.0f, noteOffset);
        }
        return arr;
    }

public:
    static inline const std::array<float, TABLE_SIZE> table = generateTable();

    static constexpr int getTableSize() { return TABLE_SIZE; }

    static inline float toValue(int x) noexcept {
        return table[x];
    }
};

class Kick {
private:
    static constexpr int size = 24;
    static constexpr int maxBufferCapacity = 24;
    static constexpr int oversampleFactor = 1;

    int sampleRate;
    float tempBuffer[maxBufferCapacity] = {};
    float oversampleBuffer[maxBufferCapacity] = {};
    
    FastRandomGenerator randomGenerator;
    
    float zero = 0.0f;
    float one = 1.0f;
    float two = 2.0f;
    static constexpr float twoLn2 = 1.3862944f;
    
    DecayEnv<EnvType::Mod, maxBufferCapacity> pitchEnv;
    AttackDecayEnv<EnvType::Amp, maxBufferCapacity> transientEnv;
    AttackDecayEnv<EnvType::Amp, maxBufferCapacity> ampEnv;
    AttackDecayEnv<EnvType::Amp, maxBufferCapacity> noiseEnv;
    
    float samplesPerSFactor;
    
    FastSvf noiseLowPassSvf;
    FastSvf noiseHighPassSvf;
    
    FastSvf oversampleSvf;
    FastSvf sub30Svf;
public:
    bool isEnvelopeFinished = false;
    bool trigger = false;
    
    float subAmp = 1.0f;
    float noiseAmp = 0.6f;
    float transientAmp = 1.0f;
    
    float totalAmp = 1.0f;
    
    float phase = 0.0f;
    float phaseSub = 0.0f;
    float phaseIncrement = 0.0f;
    
    float foldAmount = 0.5f;
    
    Kick(int sampleRate)
    : sampleRate(sampleRate) {
        samplesPerSFactor = ((2.0f * M_PI) / static_cast<float>(sampleRate)) / static_cast<float>(oversampleFactor);
        float samplesPerMs = (static_cast<float>(sampleRate) / 1000.0f) * static_cast<float>(oversampleFactor);
        pitchEnv.samplesPerMs = samplesPerMs;
        transientEnv.samplesPerMs = samplesPerMs;
        ampEnv.samplesPerMs = samplesPerMs;
        noiseEnv.samplesPerMs = samplesPerMs;

        randomGenerator = FastRandomGenerator();
        
        noiseLowPassSvf.Init();
        noiseHighPassSvf.Init();
        
        oversampleSvf.Init();
        oversampleSvf.set_f_q(0.5, 0.707f);
        
        sub30Svf.Init();
        sub30Svf.set_f_q(0.000625, 0.707f);
        
        ampEnv.isSlow = false;
        
        // Lower exponent than the 0.5 used elsewhere for this min/max Q sweep: at 0.5 the
        // resonant peak was adding extra gain right at the cutoff, which is part of why the
        // noise layer was reading louder than its amplitude knob implied.
        noiseLowPassSvf.set_f_q(0.0125f, 0.5f * pow(10.0f/0.5f, 0.2f));
        noiseHighPassSvf.set_f_q(0.004166666667f, 0.5f);
    }

    Kick(const Kick&) = delete;
    Kick& operator=(const Kick&) = delete;
    Kick(Kick&&) = delete;
    Kick& operator=(Kick&&) = delete;

    inline void updateNote(int note) {
        phaseIncrement = MidiToHz::toValue(note) * samplesPerSFactor;
    }
    
    inline void update(int ampDecayMs, int pitchDecayMs, int noiseDecayMs, bool isAmpFast) {
        ampEnv.update(2, ampDecayMs);
        transientEnv.update(2, pitchDecayMs + 4);
        pitchEnv.setDecay(pitchDecayMs);
        noiseEnv.update(2, noiseDecayMs);
        
        ampEnv.isSlow = !isAmpFast;
    }
    
    inline void updateNoiseSvf(float hiPassFreq, float lowPassFreq, float q) {
        noiseHighPassSvf.set_f_q(hiPassFreq, q);
        noiseLowPassSvf.set_f_q(lowPassFreq, q);
    }
    
    inline void process(float* destinationL, float* destinationR) {
        vDSP_vclr(destinationL, 1, size);
        vDSP_vclr(tempBuffer, 1, maxBufferCapacity);
        vDSP_vclr(oversampleBuffer, 1, maxBufferCapacity);

        if (!trigger && isEnvelopeFinished) {
            vDSP_vclr(destinationR, 1, size);
            return;
        } else if (trigger) {
            phase = 0.0f;
            phaseSub = 0.0f;
        }

        ampEnv.process(trigger, size);
        isEnvelopeFinished = ampEnv.state == 0 && pitchEnv.state == 0 && noiseEnv.state == 0;

        noiseEnv.process(trigger, size);
        
        // Transient
        transientEnv.process(trigger, size);
        pitchEnv.process(trigger, size);
        
        // 2^(env * 2): env [1→0], scaled by 2 → [2→0] semitones, then exp to get freq ratio [4→1]
        // 2^x = exp(x * ln2), combining both multiplications avoids the broken vvpowf(&two) trick
        vDSP_vsmul(pitchEnv.buffer, 1, &twoLn2, pitchEnv.buffer, 1, size);
        vvexpf(pitchEnv.buffer, pitchEnv.buffer, &size);
        vDSP_vsmul(pitchEnv.buffer, 1, &phaseIncrement, pitchEnv.buffer, 1, size);
        float cumulative = 0;
        for (int i = 0; i < size; i++) {
            cumulative += pitchEnv.buffer[i];
            pitchEnv.buffer[i] = cumulative;
        }
        
        vDSP_vsadd(pitchEnv.buffer, 1, &phase, tempBuffer, 1, size);
        phase = fmodf(tempBuffer[size - 1], 2.0f * M_PI); // Wrap to prevent overflow
        vvsinf(tempBuffer, tempBuffer, &size);
        vDSP_vsmul(tempBuffer, 1, &transientAmp, tempBuffer, 1, size);
        vDSP_vma(tempBuffer, 1, transientEnv.buffer, 1, oversampleBuffer, 1, oversampleBuffer, 1, size);
        
        // Sub
        vDSP_vramp(&phaseIncrement, &phaseIncrement, tempBuffer, 1, size);
        vDSP_vsadd(tempBuffer, 1, &phaseSub, tempBuffer, 1, size);
        phaseSub = fmodf(tempBuffer[size - 1], 2.0f * M_PI); // Wrap to prevent overflow
        vvsinf(tempBuffer, tempBuffer, &size);
        vDSP_vsmul(tempBuffer, 1, &subAmp, tempBuffer, 1, size);
        vDSP_vma(tempBuffer, 1, ampEnv.buffer, 1, oversampleBuffer, 1, oversampleBuffer, 1, size);

        // Noise
        randomGenerator.fill(tempBuffer, size);
        noiseLowPassSvf.process_low(tempBuffer, size);
        noiseHighPassSvf.process_high(tempBuffer, size);
        vDSP_vsmul(tempBuffer, 1, &noiseAmp, tempBuffer, 1, size);
        vDSP_vma(tempBuffer, 1, noiseEnv.buffer, 1, oversampleBuffer, 1, oversampleBuffer, 1, size);
        
        
        // Downsample
        oversampleSvf.process_low(oversampleBuffer, size);
        vDSP_vsmul(oversampleBuffer, oversampleFactor, &totalAmp, destinationL, 1, size);
        sub30Svf.process_high(destinationL, size);
        
        memcpy(destinationR, destinationL, size * sizeof(float));
    }
};

#endif // KICK
