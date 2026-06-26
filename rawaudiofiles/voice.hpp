//
//  Voice.hpp
//  Inertia
//
//  Converted from Voice.swift
//

#pragma once

#include <cstdint>
#include <cstring>
#include <array>
#include <algorithm>
#include <Accelerate/Accelerate.h>

#include "models.hpp"
#include "modulator.hpp"
#include "fxchain.hpp"
#include "globals.hpp"
#include "custom/lib/tables.hpp"
#include "audioconfig.hpp"
#include "./custom/samplers/samplecontainer.hpp"
#include "./custom/oscillator/wrapper.hpp"
#include "./custom/tiltednoise.hpp"
#include "./custom/kick.hpp"
#include "./plaitswrapper.hpp"
#include "synthvoice.hpp"

struct VoiceTriggerStatus {
    bool wasTriggered = false;
    int instrumentId = 0;
    int note = 0;
};

class VoiceCpp {
public:
    // --- Public state ---
    int id;
    VoiceTriggerStatus voiceTriggerStatus;
    bool isMuted = false;
    float frequency = 440.0f;
    bool isEnvelopeActive = false;
    bool forceEnvelope = false;
    int bpm = 128;

    // Post-gain linear stereo peaks for UI metering. Zeroed on silence.
    float peakL = 0.0f;
    float peakR = 0.0f;

    Lfo lfo;
    VoiceSequencerCpp& sequencer;

    PitchedSamplerFast sampler;
    GranularSampler granularSampler;
    InstrumentCpp currentInstrument;

    FastRandomGenerator fastRandomGenerator;

    // --- Constructor / Destructor ---

    VoiceCpp(int id, double sampleRate, VoiceSequencerCpp& sequencer)
        : id(id)
        , sequencer(sequencer)
        , kick(GlobalsCpp::sampleRate)
    {
        fastRandomGenerator.update(1, 0);

        currentInstrument = AudioInstrumentConfigWrapper::instance.consume().instrumentContainer.get(0);

        ampEnv.samplesPerMs = static_cast<float>(GlobalsCpp::samplesPerMs);
        modEnv.samplesPerMs = static_cast<float>(GlobalsCpp::samplesPerMs);
        synthVoice.synthFilterEnv.samplesPerMs = static_cast<float>(GlobalsCpp::samplesPerMs);
    }

    VoiceCpp(const VoiceCpp&) = delete;
    VoiceCpp& operator=(const VoiceCpp&) = delete;

    // --- Public methods ---

    void updateBpm(int newBpm) {
        bpm = newBpm;
        lfo.updateBpm(newBpm);
        modulator.updateBPM(static_cast<float>(newBpm), static_cast<float>(GlobalsCpp::sampleRate));
    }

    void setFromInstrument(
        int id,
        int* slice = nullptr,
        int* meta = nullptr,
        int* noteOverride = nullptr,
        CombinedStepCpp* noteAutomationStep = nullptr,
        float* velocityOverride = nullptr)
    {
        const InstrumentCpp& instrument = AudioInstrumentConfigWrapper::instance.consume().instrumentContainer.get(id);
        note = noteOverride ? *noteOverride : instrument.note;

        voiceTriggerStatus.instrumentId = id;
        voiceTriggerStatus.note = note;
        voiceTriggerStatus.wasTriggered = true;

        if (velocityOverride) {
            modulator.velocityModulation = *velocityOverride;
        }

        // Process modulations before reading any getValue() calls below
        modulator.process(instrument, sequencer.currentStep);

        currentInstrument = instrument;
        frequency = midiToHz(note);

        lfo.settings_ = instrument.lfoSettings;
        lfo.update();

        float ampEnvMultiplier = 1.0f + (instrument.ampEnv.durationAdjustment * 3.0f);
        ampEnv.update(
            instrument.ampEnv.attack * instrument.ampEnv.attackMultiplier * ampEnvMultiplier,
            instrument.ampEnv.hold * instrument.ampEnv.holdMultiplier * ampEnvMultiplier,
            clamp(instrument.ampEnv.release + modulator.getValue(Modulation::ModDestination::Decay)) * instrument.ampEnv.releaseMultiplier * ampEnvMultiplier
        );

        float modEnvMultiplier = 1.0f + (instrument.modEnv.durationAdjustment * 3.0f);
        modEnv.update(
            instrument.modEnv.attack * instrument.modEnv.attackMultiplier * modEnvMultiplier,
            instrument.modEnv.hold * instrument.modEnv.holdMultiplier * modEnvMultiplier,
            instrument.modEnv.release * instrument.modEnv.releaseMultiplier * modEnvMultiplier
        );

        fxChain.update(instrument);

        if (instrument.type == InstrumentTypeCpp::Drum) {
            kick.updateNote(note);
            kick.update(
                clamp(instrument.kickSettings.ampDecay + modulator.getValue(Modulation::ModDestination::PercSubAmpDecay)) * 790.0f + 10,
                clamp(instrument.kickSettings.pitchDecay + modulator.getValue(Modulation::ModDestination::PercTransientAmpDecay)) * 195.0f + 5,
                clamp(instrument.kickSettings.noiseDecay + modulator.getValue(Modulation::ModDestination::PercNoiseAmpDecay)) * 798.0f + 2,
                instrument.kickSettings.ampCurve == 0
            );

            kick.subAmp = Tables::getValue<Tables::exponentialTable>(clamp(instrument.kickSettings.ampAmount   + modulator.getValue(Modulation::ModDestination::PercSubGain)));
            kick.transientAmp = Tables::getValue<Tables::exponentialTable>(clamp(instrument.kickSettings.pitchAmount  + modulator.getValue(Modulation::ModDestination::PercTransientGain)));
            // Noise reads much louder than its raw amplitude suggests (broadband energy,
            // longer decay than the transient, and a resonant lowpass peak), so it needs
            // extra attenuation here to sit level with the sub/transient.
            kick.noiseAmp = Tables::getValue<Tables::exponentialTable>(clamp(instrument.kickSettings.noiseAmount  + modulator.getValue(Modulation::ModDestination::PercNoiseGain))) * 0.35f;

            kick.updateNoiseSvf(
                Tables::getValue<Tables::filterNormalized>(clamp(instrument.kickSettings.noiseFilterHighCutoff + modulator.getValue(Modulation::ModDestination::PercNoiseHighPass))),
                Tables::getValue<Tables::filterNormalized>(clamp(instrument.kickSettings.noiseFilterCutoff     + modulator.getValue(Modulation::ModDestination::PercNoiseLowPass))),
                Tables::getValue<Tables::stmResonanceNormalized>(instrument.kickSettings.noiseFilterQ)
            );
        } else if (instrument.type == InstrumentTypeCpp::None ||
                 instrument.type == InstrumentTypeCpp::Midi) {
            // nothing to do
        } else if (instrument.type == InstrumentTypeCpp::Synth) {
            synthVoice.tiltedNoise.update(
                Tables::getValue<Tables::exponentialTable>(instrument.noiseSettings.gain) * 0.8f,
                instrument.noiseSettings.tilt,
                Tables::getValue<Tables::filterNormalized>(instrument.noiseSettings.cutoff)
            );

            synthVoice.oscillator.wavetableIndex = clamp(instrument.synthSettings.wavetableIndex + modulator.getValue(Modulation::ModDestination::Wavetable));

            if (instrument.synthSettings.type == SynthTypeCpp::FmSynth) {
                synthVoice.oscillator.mode           = OscillatorMode::Fm;
                synthVoice.oscillator.feedback       = instrument.synthSettings.modulatorFeedback;
                synthVoice.oscillator.modulatorAmount= instrument.synthSettings.modulatorAmount;
                synthVoice.oscillator.modulatorRatio = instrument.synthSettings.modulatorRatio;
                synthVoice.oscillator.subAmp         = Tables::getValue<Tables::exponentialTable>(instrument.synthSettings.sub);
            } else if (instrument.synthSettings.type == SynthTypeCpp::SpreadSynth) {
                synthVoice.oscillator.mode = OscillatorMode::Spread;
            } else if (instrument.synthSettings.type == SynthTypeCpp::ChordSynth) {
                synthVoice.oscillator.mode = OscillatorMode::Chord;
                if (noteOverride) {
                    synthVoice.oscillator.frequencies[0] = midiToHz(*noteOverride);
                    synthVoice.oscillator.amplitudes[0]  = 0.6f;
                    synthVoice.oscillator.amplitudes[1]  = 0.0f;
                    synthVoice.oscillator.amplitudes[2]  = 0.0f;
                    synthVoice.oscillator.amplitudes[3]  = 0.0f;
                } else {
                    int currentSlice = slice ? *slice : sequencer.step.slice;
                    const auto& chord = instrument.synthSettings.chords[currentSlice];

                    for (int i = 0; i < 4; ++i) {
                        if (chord.notes[i] != -1) {
                            synthVoice.oscillator.frequencies[i] = midiToHz(chord.notes[i]);
                            synthVoice.oscillator.amplitudes[i]  = 0.6f;
                        } else {
                            synthVoice.oscillator.amplitudes[i] = 0.0f;
                        }
                    }
                }
            } else if (instrument.synthSettings.type == SynthTypeCpp::AutochordSynth) {
                synthVoice.oscillator.mode = OscillatorMode::Chord;

                int32_t currentSlice = sequencer.step.sliceAdjusted;
                if (slice) currentSlice = *slice;
                int32_t currentMeta = sequencer.step.metaAdjusted;

                ScaleInfo scale = AudioScaleConfigWrapper::instance.consume().selectedScale;

                synthVoice.oscillator.updateChord(note, currentSlice, currentMeta, scale);
            } else if (instrument.synthSettings.type == SynthTypeCpp::PolySynth) {
                synthVoice.oscillator.mode           = OscillatorMode::Chord;
                synthVoice.oscillator.amplitudes[0]  = 0.8f;
                synthVoice.oscillator.frequencies[0] = midiToHz(note);
                synthVoice.oscillator.amplitudes[1]  = 0.0f;
                synthVoice.oscillator.amplitudes[2]  = 0.0f;
                synthVoice.oscillator.amplitudes[3]  = 0.0f;

                if (noteAutomationStep) {
                    if (noteAutomationStep->activeNotes > 0) {
                        synthVoice.oscillator.frequencies[1] = midiToHz(noteAutomationStep->notes[0]);
                        synthVoice.oscillator.amplitudes[1]  = 0.8f;
                    }
                    if (noteAutomationStep->activeNotes > 1) {
                        synthVoice.oscillator.frequencies[2] = midiToHz(noteAutomationStep->notes[1]);
                        synthVoice.oscillator.amplitudes[2]  = 0.8f;
                    }
                    if (noteAutomationStep->activeNotes > 2) {
                        synthVoice.oscillator.frequencies[3] = midiToHz(noteAutomationStep->notes[2]);
                        synthVoice.oscillator.amplitudes[3]  = 0.8f;
                    }
                }
            }

            synthVoice.oscillator.update(frequency, 0.8f, instrument.synthSettings.spread);

            float synthFilterEnvMultiplier = 1.0f + (instrument.synthSettings.synthFilterEnv.durationAdjustment * 3.0f);
            synthVoice.synthFilterEnv.update(
                instrument.synthSettings.synthFilterEnv.attack * instrument.synthSettings.synthFilterEnv.attackMultiplier * synthFilterEnvMultiplier,
                instrument.synthSettings.synthFilterEnv.hold   * instrument.synthSettings.synthFilterEnv.holdMultiplier   * synthFilterEnvMultiplier,
                instrument.synthSettings.synthFilterEnv.release * instrument.synthSettings.synthFilterEnv.releaseMultiplier * synthFilterEnvMultiplier
            );
        } else if (instrument.type == InstrumentTypeCpp::Sampler) {
            if (instrument.samplerSettings.type == SamplerTypeCpp::Pitched) {
                sampler.currentSamplerIndex = instrument.samplerSettings.samplerId;
                sampler.startSample         = currentInstrument.samplerSettings.start;
                sampler.endSample           = currentInstrument.samplerSettings.end;
                sampler.direction           = ((meta ? *meta : sequencer.step.meta) == 1) ? -1 : 1;
                currentPosition             = 0;
                sampler.setNote(note);

                if (instrument.samplerSettings.envelopeType == SamplerEnvelopeTypeCpp::FadeInOutEnvelope) {
                    ampEnv.updateForFade(
                        sampler.endSample - sampler.startSample,
                        currentInstrument.samplerSettings.fadeIn,
                        currentInstrument.samplerSettings.fadeOut,
                        MidiTables::getPitchFactor(note)
                    );
                }
            }
            else if (instrument.samplerSettings.type == SamplerTypeCpp::Slicer) {
                sampler.currentSamplerIndex = instrument.samplerSettings.samplerId;
                int currentSlice            = slice ? *slice : sequencer.step.slice;
                sampler.direction           = ((meta ? *meta : sequencer.step.meta) == 1) ? -1 : 1;

                if (currentSlice < currentInstrument.samplerSettings.numSlices) {
                    sampler.startSample = currentInstrument.samplerSettings.slices[currentSlice].start;
                    if (currentSlice + 1 < currentInstrument.samplerSettings.numSlices) {
                        sampler.endSample = currentInstrument.samplerSettings.slices[currentSlice + 1].start;
                    } else {
                        sampler.setEndSample();
                    }
                } else {
                    sampler.startSample = 0;
                    sampler.endSample   = 0;
                }

                sampler.setNote(note);
                currentPosition = 0;

                if (instrument.samplerSettings.envelopeType == SamplerEnvelopeTypeCpp::FadeInOutEnvelope) {
                    ampEnv.updateForFade(
                        sampler.endSample - sampler.startSample,
                        currentInstrument.samplerSettings.fadeIn,
                        currentInstrument.samplerSettings.fadeOut,
                        MidiTables::getPitchFactor(note)
                    );
                }
            } else if (instrument.samplerSettings.type == SamplerTypeCpp::Granular) {
                granularSampler.setSample(
                    instrument.samplerSettings.samplerId,
                    currentInstrument.samplerSettings.start,
                    ((meta ? *meta : sequencer.step.meta) == 1) ? -1 : 1
                );
                granularSampler.setEnd(currentInstrument.samplerSettings.end);
                granularSampler.update(
                    modulator.get(Modulation::ModDestination::GranularSize,     currentInstrument.samplerSettings.grainSize),
                    modulator.get(Modulation::ModDestination::GranularSpray,    currentInstrument.samplerSettings.spray),
                    modulator.get(Modulation::ModDestination::GranularEnvelope, currentInstrument.samplerSettings.grainEnv)
                );
                currentPosition = 0;
                granularSampler.setNote(note);
            } else if (instrument.samplerSettings.type == SamplerTypeCpp::GranularSlicer) {
                int currentSlice = slice ? *slice : sequencer.step.slice;
                if (currentSlice < currentInstrument.samplerSettings.numSlices) {
                    granularSampler.setSample(
                        instrument.samplerSettings.samplerId,
                        currentInstrument.samplerSettings.slices[currentSlice].start,
                        ((meta ? *meta : sequencer.step.meta) == 1) ? -1 : 1
                    );
                    if (currentSlice + 1 < currentInstrument.samplerSettings.numSlices) {
                        granularSampler.setEnd(currentInstrument.samplerSettings.slices[currentSlice + 1].start);
                    } else {
                        granularSampler.setEndSample();
                    }
                } else {
                    granularSampler.setSample(
                        instrument.samplerSettings.samplerId,
                        0,
                        ((meta ? *meta : sequencer.step.meta) == 1) ? -1 : 1
                    );
                    granularSampler.setEnd(0);
                }
                granularSampler.update(
                    modulator.get(Modulation::ModDestination::GranularSize,     currentInstrument.samplerSettings.grainSize),
                    modulator.get(Modulation::ModDestination::GranularSpray,    currentInstrument.samplerSettings.spray),
                    modulator.get(Modulation::ModDestination::GranularEnvelope, currentInstrument.samplerSettings.grainEnv)
                );
                currentPosition = 0;
                granularSampler.setNote(note);
            }
        }
        else {
            // Default / macro (plaits)
            const auto& conf = currentInstrument.plaitsSettings;
            plaitsWrapper.update(
                static_cast<float>(conf.model),
                conf.crossfade,
                static_cast<float>(note),
                conf.harmonics,
                conf.timbre,
                conf.morph,
                conf.decay,
                conf.color,
                conf.model == 13 ? conf.pitchDecay : 0.0f
            );
        }
    }

    void fillSamplerPosition(SamplerVoicePosition& out) const {
        out.position1 = 0;
        out.position2 = 0;
        if (!isEnvelopeActive || currentInstrument.type != InstrumentTypeCpp::Sampler) {
            out.instrumentId = -1;
            return;
        }
        
        out.instrumentId = (int32_t)voiceTriggerStatus.instrumentId;
        const bool isGranular =
            currentInstrument.samplerSettings.type == SamplerTypeCpp::Granular ||
            currentInstrument.samplerSettings.type == SamplerTypeCpp::GranularSlicer;
        if (isGranular) {
            out.position1 = (int32_t)granularSampler.grains[0].currentReadPos;
            out.position2 = (int32_t)granularSampler.grains[1].currentReadPos;
        } else {
            out.position1 = sampler.currentReadPos;
        }
    }

    void startEnvelope(bool force = false) {
        forceEnvelope   = force;
        isEnvelopeActive= true;
        ampEnv.reset();
        modEnv.reset();
        synthVoice.reset();
    }

    void stopEnvelope() {
        ampEnv.falloff();
    }

    bool shouldRun(float random) {
        if (random >= 0.99f) return true;
        return fastRandomGenerator.nextFloat() > random;
    }

    void process(
        int frameSize,
        float voiceGain,
        float* voiceBufferL,
        float* voiceBufferR,
        float* delayBufferL,
        float* delayBufferR,
        float* multitapBufferL,
        float* multitapBufferR,
        float* reverbBufferL,
        float* reverbBufferR)
    {
        if (sequencer.shouldTriggerEnvelope) {
            startEnvelope();
        }

        modulator.sectionModulation = sequencer.laneTwo;

        if (sequencer.wasUpdated) {
            modulator.laneOneData = sequencer.currentPage->automation.rawValues.data();

            if (sequencer.step.status) {
                const auto& noteAutomation = sequencer.currentPage->steps[sequencer.currentStep];
                int noteOverrideVal        = noteAutomation.note;
                int* noteOverride          = noteAutomation.isNoteActive ? &noteOverrideVal : nullptr;

                modulator.lfoModulation      = lfo.process();
                modulator.velocityModulation = sequencer.step.velocity;

                // setFromInstrument calls modulator.process() internally
                setFromInstrument(
                    sequencer.step.instrumentId,
                    nullptr, nullptr,
                    noteOverride,
                    const_cast<CombinedStepCpp*>(&noteAutomation)
                );
                voiceTriggerStatus.wasTriggered = true;
            } else {
                modulator.lfoModulation = lfo.process();
                modulator.process(currentInstrument, sequencer.currentStep);
                voiceTriggerStatus.wasTriggered = false;
            }
        } else {
            modulator.lfoModulation         = lfo.process();
            voiceTriggerStatus.wasTriggered = false;
        }

        const bool shouldTrigger = sequencer.shouldTriggerEnvelope || forceEnvelope;

        if (currentInstrument.type == InstrumentTypeCpp::Macro) {
            isEnvelopeActive = shouldTrigger || plaitsWrapper.isLpgActive();
        } else if (currentInstrument.type == InstrumentTypeCpp::Drum) {
            isEnvelopeActive = shouldTrigger || !kick.isEnvelopeFinished;
        } else {
            isEnvelopeActive = shouldTrigger || !ampEnv.isComplete;
        }

        if (isMuted ||
            currentInstrument.type == InstrumentTypeCpp::None ||
            !isEnvelopeActive)
        {
            forceEnvelope = false;
            ampEnv.falloff();
            peakL = peakR = 0.0f;
            return;
        }

        modulator.increment(static_cast<float>(frameSize));

        modEnv.process(shouldTrigger, frameSize);
        modEnvAmount = modEnv.buffer[0];

        if (currentInstrument.type == InstrumentTypeCpp::Macro) {
            plaitsWrapper.updateModulations(
                modulator.getValue(Modulation::ModDestination::Harmonics),
                modulator.getValue(Modulation::ModDestination::Timbre),
                modulator.getValue(Modulation::ModDestination::Morph)
            );
            plaitsWrapper.updateInternal(
                clamp(currentInstrument.plaitsSettings.decay + modulator.getValue(Modulation::ModDestination::MacroDecay)),
                clamp(currentInstrument.plaitsSettings.color + modulator.getValue(Modulation::ModDestination::MacroColor))
            );
            plaitsWrapper.updateTrigger(shouldTrigger ? 1 : 0);
            plaitsWrapper.process(tempVoiceBufferL, frameSize);
            std::memcpy(tempVoiceBufferR, tempVoiceBufferL, frameSize * sizeof(float));
        } else if (currentInstrument.type == InstrumentTypeCpp::Drum) {
            kick.trigger = shouldTrigger;
            kick.totalAmp = 1.0f;
            kick.process(tempVoiceBufferL, tempVoiceBufferR);
        } else if (currentInstrument.type == InstrumentTypeCpp::Synth) {
            synthVoice.process(tempVoiceBufferL, tempVoiceBufferR, frameSize, currentInstrument, modulator, shouldTrigger, modEnvAmount);

            ampEnv.process(shouldTrigger, frameSize);
            ampEnv.applyTo(tempVoiceBufferL, tempVoiceBufferR, frameSize);

            std::memcpy(tempVoiceBufferR, tempVoiceBufferL, frameSize * sizeof(float));
        } else if (currentInstrument.type == InstrumentTypeCpp::Sampler) {
            const bool isGranular =
                currentInstrument.samplerSettings.type == SamplerTypeCpp::Granular ||
                currentInstrument.samplerSettings.type == SamplerTypeCpp::GranularSlicer;

            if (isGranular) {
                granularSampler.update(
                    modulator.get(Modulation::ModDestination::GranularSize,     currentInstrument.samplerSettings.grainSize),
                    modulator.get(Modulation::ModDestination::GranularSpray,    currentInstrument.samplerSettings.spray),
                    modulator.get(Modulation::ModDestination::GranularEnvelope, currentInstrument.samplerSettings.grainEnv)
                );
                granularSampler.process(shouldTrigger, tempVoiceBufferL, tempVoiceBufferR, frameSize);
            } else {
                sampler.process(currentPosition, tempVoiceBufferL, tempVoiceBufferR, frameSize);
            }
            currentPosition += frameSize;

            float sampleGain = Tables::getValue<Tables::exponentialMidpoint>(currentInstrument.samplerSettings.gain);
            vDSP_vsmul(tempVoiceBufferL, 1, &sampleGain, tempVoiceBufferL, 1, frameSize);
            vDSP_vsmul(tempVoiceBufferR, 1, &sampleGain, tempVoiceBufferR, 1, frameSize);

            ampEnv.process(shouldTrigger, frameSize);
            ampEnv.applyTo(tempVoiceBufferL, tempVoiceBufferR, frameSize);
        }

        fxChain.apply(
            currentInstrument,
            modulator,
            sequencer.didStep,
            sequencer.currentStep,
            modEnvAmount,
            bpm,
            tempVoiceBufferL,
            tempVoiceBufferR,
            frameSize
        );

        auto mixInto = [&](float* dstL, float* dstR, float amount) {
            float gain = amount * voiceGain;
            vDSP_vsma(tempVoiceBufferL, 1, &gain, dstL, 1, dstL, 1, frameSize);
            vDSP_vsma(tempVoiceBufferR, 1, &gain, dstR, 1, dstR, 1, frameSize);
        };

        float mainGain = Tables::getValue<Tables::exponentialTable>(clamp(currentInstrument.mixerSettings.gainAmount + modulator.getValue(Modulation::ModDestination::Gain))) * voiceGain;

        // Need to do this here as the voice buffer is cumulative - I guess we could change that in the future tbh
        float rawPeakL = 0.0f, rawPeakR = 0.0f;
        vDSP_maxmgv(tempVoiceBufferL, 1, &rawPeakL, frameSize);
        vDSP_maxmgv(tempVoiceBufferR, 1, &rawPeakR, frameSize);
        peakL = rawPeakL * mainGain;
        peakR = rawPeakR * mainGain;

        vDSP_vsma(tempVoiceBufferL, 1, &mainGain, voiceBufferL, 1, voiceBufferL, 1, frameSize);
        vDSP_vsma(tempVoiceBufferR, 1, &mainGain, voiceBufferR, 1, voiceBufferR, 1, frameSize);
        float reverbSendRaw = clamp(currentInstrument.mixerSettings.reverbSend + modulator.getValue(Modulation::ModDestination::Reverb));
        if (reverbSendRaw > 0.0f)
            mixInto(reverbBufferL, reverbBufferR, Tables::getValue<Tables::exponentialTable>(reverbSendRaw));

        float delaySendRaw = clamp(currentInstrument.mixerSettings.delaySend + modulator.getValue(Modulation::ModDestination::Delay));
        if (delaySendRaw > 0.0f)
            mixInto(delayBufferL, delayBufferR, Tables::getValue<Tables::exponentialTable>(delaySendRaw));

        float multitapSendRaw = clamp(currentInstrument.mixerSettings.multitapSend + modulator.getValue(Modulation::ModDestination::Multitap));
        if (multitapSendRaw > 0.0f)
            mixInto(multitapBufferL, multitapBufferR, Tables::getValue<Tables::exponentialTable>(multitapSendRaw));

        forceEnvelope = false;
    }

private:

    // --- Audio components ---
    SynthVoiceDsp     synthVoice;
    Kick              kick;
    FxChainCpp        fxChain;
    AttackHoldDecayEnv<EnvType::Amp, 24>       ampEnv;
    AttackHoldDecayEnv<EnvType::Mod, 24>    modEnv;
    PlaitsWrapper       plaitsWrapper;
    ModulatorCpp      modulator;

    // --- Temp buffers ---
    float tempVoiceBufferL[GlobalsCpp::frameSize] = {};
    float tempVoiceBufferR[GlobalsCpp::frameSize] = {};

    // --- State ---
    int   note             = 0;
    float pitchEnvelopeAmount = 0.0f;
    float modEnvAmount     = 0.0f;
    int   currentPosition  = 0;
    int   granularPosition = 0;
    int   granularEnd      = 0;

    // --- Private helpers ---

    inline float midiToHz(int n) const {
        return MidiTables::getHz(n);
    }

    inline float clamp(float v) const {
        return std::min(1.0f, std::max(v, 0.0f));
    }
};
