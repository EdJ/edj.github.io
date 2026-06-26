//
//  models.hpp
//  Jolt
//
//  Created by Ed James on 20/03/2026. (Generated)
//

#pragma once

#import <array>
#include <cstdint>
#import "custom/fastrandomgenerator.hpp"
#import "custom/lfo.hpp"
#include "dsp_macros.hpp"
#include "globals.hpp"

// Must match AudioWrapper.numberOfVoices in Swift
#ifndef NUMBER_OF_VOICES
#define NUMBER_OF_VOICES 8
#endif

static constexpr int TotalNumberOfVoices = 8; // match AudioWrapper::numberOfVoices

struct NoiseSettingsCpp {
    float gain = 0.0f;
    float decay = 0.12f;
    float tilt = 0.8f;
    float cutoff = 0.25f;
};

struct AHRCpp {
    float attack = 0.025f;
    float attackMultiplier = 200.0f;

    float hold = 0.1f;
    float holdMultiplier = 600.0f;

    float release = 0.5f;
    float releaseMultiplier = 600.0f;

    float durationAdjustment = 0.0f;
    float end = 0.0f;
};

struct PitchEnvCpp {
    int semitones = 0;
    int decay = 12;
};

struct FilterSettingsCpp {
    float cutoff = 1.0f;
    float resonance = 0.2f;
    int filterType = 0;
};

struct MixerSettingsCpp {
    float delaySend = 0;
    float multitapSend = 0;
    float reverbSend = 0;
    float gainAmount = 1;
    float pan = 0.5f;
    float width = 0.5f;
};

struct SliceDetailsCpp {
    int id = 0;
    int start = 0;
    int end = 0;
};

enum SamplerTypeCpp {
    Pitched = 0,
    Slicer,
    Granular,
    GranularSlicer,
    SAMPLERTYPECPP_SIZE
};

enum SamplerEnvelopeTypeCpp {
    FixedEnvelope = 0,
    FadeInOutEnvelope,
    SAMPLERENVELOPETYPECPP_SIZE
};

struct SamplerSettingsCpp {
    int type = 0;

    int start = 0;
    int end = 0;
    int samplerId = 0;
    bool hasSampleSelected = false;
    float gain = 0.5f;

    int envelopeType = 0;
    int fadeIn = 0;
    int fadeOut = 0;

    WITH_ARRAY(slices, SliceDetailsCpp, 16)
    int numSlices = 0;
    int numberOfSlices = 4;
    bool hasBeenSliced = false;

    float grainSize = 0.2f;
    float spray = 0;
    float grainEnv = 0;
};

struct ChordCpp {
    int root = 0;
    WITH_ARRAY(notes, int, 4, -1, -1, -1, -1)
};

enum SynthTypeCpp {
    FmSynth = 0,
    SpreadSynth,
    ChordSynth,
    AutochordSynth,
    PolySynth,
    SYNTHTYPECPP_SIZE
};

struct SynthSettingsCpp {
    int type = 0;
    float wavetableIndex = 0;
    float spread = 0;

    float modulatorAmount = 0;
    float modulatorRatio = 2;
    float modulatorFeedback = 0.2f;
    float sub = 0;

    WITH_ARRAY(chords, ChordCpp, 8)

    FilterSettingsCpp synthFilterSettings = {};
    AHRCpp synthFilterEnv = {};
};

struct PlaitsSettingsCpp {
    int model = 0;
    float crossfade = 0;
    float timbre = 0.5f;
    float morph = 0.5f;
    float harmonics = 0.5f;
    float decay = 0.1f;
    float color = 0.1f;
    float pitchDecay = 0;
    bool hasModelSelected = false;
};

struct EqSettingsCpp {
    float lowGain = 0.83f;
    float midGain = 0.83f;
    float highGain = 0.83f;

    float lowCrossover = 0.2f;
    float highCrossover = 0.7f;
};

struct FxSettingsCpp {
    float overdrive = 0;
    float saturation = 0;
    float saturationStrong = 0;
    float saturationGain = 1;

    float phaserDepth = 0.3f;
    float phaserRate = 0.1f;
    float phaserFeedback = 0.3f;
    int phaserStages = 4;
    float phaserAmount = 1;

    float chorus = 0.5f;
    float chorusDepth = 0.5f;

    int bitDepth = 16;
    int sampleRateReduction = 0;

    float erosionAmount = 0;
    float erosionCutoff = 0.5f;
    float erosionQ = 0.25f;

    float tapeDrive = 0;
    float tapeWarmth = 0;
    float tapeHysteresis = 0;

    float tapeWow = 0;
    float tapeWowDrift = 0;
    float tapeFlutter = 0;
    float tapeFlutterDrift = 0;

    float gaterHold = 0.5f;
    float gaterDecay = 0.1f;
    float gaterDepth = 1.0f;
    WITH_ARRAY(gaterSteps, bool, 16)
    WITH_ARRAY(gaterHolds, int, 16)

    EqSettingsCpp eqSettings;

    WITH_ARRAY(fxChain, int, 16)
    int fxChainCount = 0;
};

enum FrequencyBandCpp: int {
    low = 0,
    lowMid,
    mid,
    high,
    highAndMid,
    lowAndLowMid,
    off,
    custom,
    FREQUENCYBANDCPP_SIZE
};

struct BandSettingsCpp {
    int segment = 0;
    float customBandHigh = 0.2f;
    float customBandLow = 0.7f;
    float dryWet = 0.5f;
};

struct MidiSettingsCpp {
    int destination = 0;
    int channel = 0;
};

struct KickSettingsCpp {
    int note = 64;

    float ampDecay = 0.4f;
    float ampAmount = 1;
    int ampCurve = 0;

    float pitchDecay = 0.1f;
    float pitchAmount = 0.5f;

    float noiseDecay = 0.2f;
    float noiseAmount = 0.5f;

    float noiseFilterCutoff = 1;
    float noiseFilterHighCutoff = 0.0125f;
    float noiseFilterQ = 0.1f;
};

enum InstrumentTypeCpp: int {
    None = 0,
    Synth,
    Sampler,
    Macro,
    Drum,
    Midi,
    InstrumentTypeCpp_SIZE
};

namespace Modulation {
    enum ModSource: int {
        ModEnv = 0,
        LaneOne,
        LaneTwo,
        Lfo,
        Velocity,
        SynthFilterEnv,
        MODSOURCE_COUNT
    };

    enum ModDestination: int {
        Filter = 0,
        Gain,
        FmModulator,
        Delay,
        Reverb,
        Timbre,
        Harmonics,
        Morph,
        Decay,
        Overdrive,
        ChorusAmount,
        ChorusDepth,
        Pan,
        Wavetable,
        MacroDecay,
        MacroColor,
        CombDelay,
        CombFeedback,
        FilterResonance,
        Multitap,
        Saturation,
        SaturationStrong,
        PhaserDepth,
        PhaserRate,
        PhaserFeedback,
        PercSubGain,
        PercSubAmpDecay,
        PercTransientGain,
        PercTransientAmpDecay,
        PercNoiseGain,
        PercNoiseAmpDecay,
        PercNoiseLowPass,
        PercNoiseHighPass,
        SynthSpread,
        FxGaterHold,
        FxGaterDecay,
        GranularSize,
        GranularSpray,
        GranularEnvelope,
        FxGaterDepth,
        SynthFilter,
        MODDESTINATION_COUNT
    };
}

struct ModMappingCpp {
    int32_t source;
    int32_t destination;
    float   amount;
};

struct ModMappingsCpp {
    static constexpr int maxCount =
        Modulation::ModSource::MODSOURCE_COUNT *
        Modulation::ModDestination::MODDESTINATION_COUNT;
    WITH_ARRAY(mappings, ModMappingCpp, maxCount)
    int32_t count = 0;
};

struct InstrumentCpp {
    int id = 0;
    int type = 0;
    int note = 0;

    SamplerSettingsCpp samplerSettings = {};
    SynthSettingsCpp synthSettings = {};
    KickSettingsCpp kickSettings = {};
    AHRCpp ampEnv = {};
    AHRCpp modEnv = {};
    PitchEnvCpp pitchEnv = {};
    MixerSettingsCpp mixerSettings = {};
    FilterSettingsCpp filterSettings = {};
    NoiseSettingsCpp noiseSettings = {};
    PlaitsSettingsCpp plaitsSettings = {};
    MidiSettingsCpp midiSettings = {};
    FxSettingsCpp fxSettings = {};
    BandSettingsCpp bandSettings = {};
    LfoSettings lfoSettings = {};

    ModMappingsCpp modMappings = {};
};

struct InstrumentContainerCpp {
    WITH_ARRAY(instruments, InstrumentCpp, 32)

    InstrumentCpp emptyInstrument = {};

    const InstrumentCpp& get(int id) const {
        if (id == -1) {
            return emptyInstrument;
        }

        return instruments[id];
    }
};

struct CombinedStepCpp {
    int id = 0;
    int ratchet = 0;
    bool hasRatchets = false;
    bool status = false;
    float random = 1.0f;
    int note = 60;
    float velocity = 0.5f;
    int instrumentId = 0;
    int slice = 0;
    int sliceAdjusted = 0;
    int meta = 0;
    int metaAdjusted = 0;

    std::array<int, 4> notes = {};
    int activeNotes = 0;
    bool isNoteActive = false;
    bool isPoly = false;
};

struct ParameterAutomationLaneCpp {
    std::array<uint8_t, GlobalsCpp::automationLaneSize> rawValues = {};
};

struct PageOfStepsCpp {
    ParameterAutomationLaneCpp automation = {};
    std::array<CombinedStepCpp, 16> steps = {};

    // Bypass Swift C++ interop copy-in/copy-out for std::array fields.
    uint8_t*         automationRawData() { return automation.rawValues.data(); }
    CombinedStepCpp* stepsData()         { return steps.data(); }
};

struct SequencerTrackCpp {
    int bpm = 128;
    int swing = 0;
    std::array<PageOfStepsCpp, 8> pages = {};
    int numberOfPages = 1;
    std::array<int, 4> ratchetDurations = {0,0,0,0};
};

class VoiceSequencerCpp {
private:
    FastRandomGenerator randomGenerator = {};

    inline bool randomlyTrigger(float stepRandom) {
        return stepRandom >= 0.099f || stepRandom >= randomGenerator.nextInRange(0, 1);
    }

public:
    int nextRatchetTime = 0;
    int currentStep = 0;

    int pointInSequence = 0;
    int currentStepTime = 0;
    bool isRunning = false;
    bool isMuted = false;
    bool justStarted = true;
    bool justStepped = true;

    bool didStep = false;
    bool didStepTrigger = false;

    bool wasUpdated = false;
    bool shouldTriggerEnvelope = false;

    float laneTwo = 0.0f;

    CombinedStepCpp step = {};
    const PageOfStepsCpp* currentPage = nullptr;
    SequencerTrackCpp track = {};

    void update(const SequencerTrackCpp& t) {
        track = t;
    }

    void play() {
        isRunning = true;
        reset();
    }

    void stop() {
        isRunning = false;
    }

    void reset() {
        justStarted = true;
        currentStep = 0;
        pointInSequence = 0;
        nextRatchetTime = 0;
        currentStepTime = 0;
        wasUpdated = false;
        shouldTriggerEnvelope = false;
        currentPage = &track.pages[0];
    }

    void stepAt(int time) {
        if (!isRunning) {
            return;
        }

        didStep = justStepped;

        if (justStepped) {
            didStep = true;

            currentPage = &track.pages[pointInSequence % track.numberOfPages];
            step = currentPage->steps[currentStep];
            wasUpdated = !isMuted;
            shouldTriggerEnvelope = !isMuted && step.status && randomlyTrigger(step.random);
            didStepTrigger = shouldTriggerEnvelope;

            if (step.status && step.hasRatchets) {
                nextRatchetTime = currentStepTime + track.ratchetDurations[step.ratchet - 2];
            }

            justStepped = false;
            return;
        }

        if (!isMuted && step.status && step.hasRatchets && didStepTrigger) {
            if (time >= nextRatchetTime) {
                nextRatchetTime = nextRatchetTime + track.ratchetDurations[step.ratchet - 2];
                wasUpdated = false;
                shouldTriggerEnvelope = true;
                return;
            }
        }

        wasUpdated = false;
        shouldTriggerEnvelope = false;
    }
};

struct DelaySettingsCpp {
    int delayBeats = 4;
    float delayFeedback = 0.4f;
    float delayReverbSend = 0.0f;
    bool delayPingPong = false;

    int getMs(int bpm) const {
        return static_cast<int>((60000.0f / static_cast<float>(bpm)) * (static_cast<float>(delayBeats) * 0.25f));
    }
};

struct MultiTapSettingsCpp {
    int delayBeats = 8;
    WITH_ARRAY(tapGains, float, 8, 0.5f, 0.25f, 0.1f, 0.5f, 0.0f, 0.25f, 0.0f, 0.1f)
    float delayReverbSend = 0.0f;

    int getMs(int bpm) const {
        return static_cast<int>((60000.0f / static_cast<float>(bpm)) * (static_cast<float>(delayBeats) * 0.25f));
    }
};

struct ReverbSettingsCpp {
    float tail = 0.8f;
    float diffusion = 0.9f;
    float lowPass = 0.7f;
    float highPass = 0.25f;
    float preDelay = 0.5f;
};

struct CompressorSettingsCpp {
    float threshold = 0.5f;
    float ratio = 2.0f;
    float preGain = 1.0f;
    float gain = 1.0f;
};

struct SendSettingsCpp {
    DelaySettingsCpp delaySettings;
    MultiTapSettingsCpp multitapSettings;
    ReverbSettingsCpp reverbSettings;
    CompressorSettingsCpp compressorSettings;
};

struct MasterMixerSettingsCpp {
    WITH_ARRAY(gains, float, NUMBER_OF_VOICES)
    WITH_ARRAY(mutes, bool, NUMBER_OF_VOICES)
    WITH_ARRAY(solos, bool, NUMBER_OF_VOICES)

    // Return level for the global FX buses (0 = delay, 1 = reverb, 2 = multitap).
    // Default 1.0 (unity) preserves previous behavior, which had no return control.
    WITH_ARRAY(fxReturnGains, float, 3)
    WITH_ARRAY(fxMutes, bool, 3)
    WITH_ARRAY(fxSolos, bool, 3)

    // Applied after the tanh limiter. Linear 0-1, default unity.
    float masterGain = 1.0f;

    MasterMixerSettingsCpp() {
        gains.fill(0.8f);
        mutes.fill(false);
        solos.fill(false);
        fxReturnGains.fill(1.0f);
        fxMutes.fill(false);
        fxSolos.fill(false);
    }
};

struct SidechainSettingsCpp {
    int triggerVoice = 0;
    WITH_ARRAY(voices, bool, NUMBER_OF_VOICES)
    WITH_ARRAY(sidechainFx, bool, 3)
    float amount = 0.0f;
    int attackMs = 12;
    int decayMs = 48;

    enum SidechainEffectIndex : int {
        Reverb = 0,
        Delay = 1,
        Multitap = 2,
    };

    SidechainSettingsCpp() {
        voices.fill(false);
    }
};

struct FxMacroSettingCpp {
    int id = 0;
    int type = 0; // PerformanceFx raw value
    float min = 0.0f;
    float max = 1.0f;
};

struct FxMacroCpp {
    int id = 0;
    WITH_ARRAY(fx, FxMacroSettingCpp, 6)
    int fxCount = 0;

    enum PerformanceFx: int {
        reverb = 0,
        delay = 1,
        lowPass = 2,
        highPass = 3,
        beatRepeat = 4,
        phaser = 5,
        gater = 6,
    };
};

struct FxMacroSettingsCpp {
    WITH_ARRAY(macros, FxMacroCpp, 8)
    int macrosCount = 0;
};

struct SongFxSettingsCpp {
    SendSettingsCpp sendSettings;
    SidechainSettingsCpp sidechain;
    MasterMixerSettingsCpp mixer;
    FxMacroSettingsCpp fxMacros;
};

struct LoopGroupFxSettingsCpp {
    float lowPass = 1;
    float lowPassModulation = 0;
    float highPass = 0;
    float highPassModulation = 0;
    float reverb = 0;
    float reverbModulation = 0;
    float delay = 0;
    float delayModulation = 0;
};
