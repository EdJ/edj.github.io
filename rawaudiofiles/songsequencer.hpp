//
//  sonsequencer.hpp
//  Jolt
//
//  Created by Ed James on 06/05/2026.
//

#pragma once

#include <array>
#include <optional>
#include <cstring>
#include <algorithm>
#include <type_traits>

#include "uuidkey.hpp"
#include "audioconfig.hpp"
#include "audiohandle.h"

// --------------------------------------------------------------------------
// Constants
// --------------------------------------------------------------------------
static constexpr int StepsPerPage = 16;
static constexpr UUIDKey EmptyUUID{};

// --------------------------------------------------------------------------
// SongSequencerCpp
// --------------------------------------------------------------------------
class SongSequencerCpp {
public:
    UUIDKey currentLoopGroup = {};
    SequencerDataCpp sequencerData;
    std::array<VoiceSequencerCpp, MAX_VOICES> voices;
    int voiceCount = 0;

    float laneTwoOverride = 0.0f;
    float groupSamplePosition = 0.0f;
    float currentGroupAutomation = 0.0f;

    int currentGroup = 0;
    int currentGroupLoops = 0;
    int currentSegment = 0;
    int currentStepWithinSegment = 0;
    int currentStep = 0;
    int loopGroupSteps = StepsPerPage;

    std::optional<int> loopedGroupIndex = std::nullopt;

    bool justStepped = true;
    int64_t nextStepTime = 0;
    int64_t lastStepTime = 0;
    int64_t lastTimeStepCalled = 0;

    int stepDuration = 0;

    // --- Construction / update --------------------------------------------

    explicit SongSequencerCpp(const SequencerDataCpp& data)
        : sequencerData(data)
    {
        static_assert(std::is_trivially_copyable<SequencerDataCpp>::value,
                      "SequencerDataCpp must stay trivially copyable for update() memcpy");

        voiceCount = MAX_VOICES;

        for (int i = 0; i < voiceCount; ++i) {
            VoiceSequencerCpp seq{};
            seq.track.bpm   = static_cast<int32_t>(sequencerData.bpm);
            seq.track.swing = static_cast<int32_t>(sequencerData.swing);
            voices[i] = seq;
        }

        configureSong();
    }

    void update(const SequencerDataCpp& data) {
        std::memcpy(&sequencerData, &data, sizeof(SequencerDataCpp));
        configureSong();
    }

    VoiceSequencerCpp& getStepSequencer(int voice) {
        return voices[voice];
    }

    // --- Transport --------------------------------------------------------

    void play(int groupOverride, bool hasGroupOverride) {
        if (hasGroupOverride) {
            if (isRunning && groupOverride == currentGroup) {
                return;
            }
        }

        isRunning = true;
        reset(groupOverride, hasGroupOverride);

        for (int v = 0; v < voiceCount; ++v) {
            voices[v].play();
        }
    }

    void stop() {
        isRunning = false;
        for (int v = 0; v < voiceCount; ++v) {
            voices[v].stop();
        }
        reset(0, false);
    }

    void reset(int groupOverride, bool hasGroupOverride) {
        justStarted = true;
        nextStepTime = 0;

        currentGroup = hasGroupOverride ? groupOverride : 0;
        currentGroupLoops = 0;
        currentSegment = 0;
        currentStepWithinSegment = 0;
        currentStep = 0;

        configureSong();
    }

    // --- Per-sample / per-block step function ------------------------------

    void step(int64_t time) {
        if (!isRunning) return;

        groupSamplePosition += static_cast<float>(time - lastTimeStepCalled);
        lastTimeStepCalled = time;

        if (justStarted) {
            if (sequencerData.hasQueued) {
                currentGroupLoops = 0;
                currentGroup = 0;
                groupSamplePosition = 0.0f;

                swapQueued();
                useGroup();
            }

            groupSamplePosition = 0.0f;

            justStarted = false;
            justStepped = true;

            for (int v = 0; v < voiceCount; ++v) {
                voices[v].justStarted     = true;
                voices[v].justStepped     = true;
                voices[v].currentStepTime = static_cast<int32_t>(time);
                voices[v].currentStep     = 0;
                voices[v].pointInSequence = 0;
            }

            nextStepTime = time + stepDurations[currentStep % StepsPerPage];
            updatePosition();

        } else if (time >= nextStepTime) {
            lastStepTime = nextStepTime;
            justStepped  = true;

            currentStep              = (currentStep + 1) % loopGroupSteps;
            currentStepWithinSegment = currentStep % StepsPerPage;
            currentSegment           = currentStep / StepsPerPage;

            // Handle queued at wrap-around
            if (currentStep == 0 && sequencerData.hasQueued) {
                currentGroupLoops = 0;
                currentGroup = 0;
                groupSamplePosition = 0.0f;

                swapQueued();
                useGroup();
            }

            // Group advancement
            if (currentStep == 0 && !sequencerData.hasLoopPattern && !sequencerData.isPerforming) {
                const int repeats = static_cast<int>(sequencerData.loopGroups[currentGroup].repeats);
                currentGroupLoops = (currentGroupLoops + 1) % repeats;

                if (currentGroupLoops == 0) {
                    if (loopedGroupIndex.has_value()) {
                        currentGroup = loopedGroupIndex.value();
                    } else {
                        currentGroup = (currentGroup + 1) % sequencerData.loopGroupCount;
                    }

                    groupSamplePosition = 0.0f;
                }

                useGroup();

            } else if (currentStep == 0 &&
                       ((sequencerData.hasLoopPattern && !sequencerData.hasCurrentlySelectedLoopGroup) ||
                        sequencerData.isPerforming)) {
                groupSamplePosition = 0.0f;
            }

            for (int v = 0; v < voiceCount; ++v) {
                voices[v].justStepped     = true;
                voices[v].currentStepTime = static_cast<int32_t>(nextStepTime);
                voices[v].currentStep     = static_cast<int32_t>(currentStepWithinSegment);
                voices[v].pointInSequence = static_cast<int32_t>(currentSegment);
            }

            nextStepTime = nextStepTime + stepDurations[currentStep % StepsPerPage];
            updatePosition();
        } else {
            justStepped = false;
        }

        currentGroupAutomation = (laneTwoOverride != 0.0f) ? laneTwoOverride : calculateGroupAutomation();
        for (int v = 0; v < voiceCount; ++v) {
            voices[v].stepAt(static_cast<int32_t>(time));
            voices[v].laneTwo = currentGroupAutomation;
        }
    }

private:
    bool isRunning = false;
    bool justStarted = false;
    std::array<int, StepsPerPage> stepDurations{};
    std::array<int, 4> ratchetDurations{};

    std::array<PageOfStepsCpp, MAX_PAGES * MAX_VOICES * MAX_GROUPS> allGroupPages = {};
    std::array<UUIDKey, MAX_PAGES * MAX_VOICES * MAX_GROUPS> allGroupPatternIds = {};
    std::array<int, MAX_PAGES * MAX_VOICES * MAX_GROUPS> allGroupPageIds = {};

    std::array<PageOfStepsCpp, 1> cleanPages = {};

    // Centralised flat index into the group/voice/page arrays.
    static int flatIndex(int g, int v, int p) {
        return g * MAX_VOICES * MAX_PAGES + v * MAX_PAGES + p;
    }

    // --- LaneTwo array lookup --------------------------------------------

    inline float calculateGroupAutomation() const {
        if (currentGroup < 0 || currentGroup >= MAX_GROUPS) return 0.0f;

        const float totalSamples = static_cast<float>(stepDuration)
                                 * static_cast<float>(sequencerData.allGroupDurations[currentGroup])
                                 * static_cast<float>(StepsPerPage);
        if (totalSamples <= 0.0f) return 0.0f;

        constexpr int LaneSize = GlobalsCpp::automationLaneSize;
        const auto& rawValues = sequencerData.loopGroups[currentGroup].laneTwoLane.rawValues;

        float wrappedPosition = std::fmod(groupSamplePosition, totalSamples);
        float pos = (wrappedPosition / totalSamples) * static_cast<float>(LaneSize);
        pos = std::min(pos, static_cast<float>(LaneSize - 1));

        int i1 = static_cast<int>(pos);
        int i2 = std::min(i1 + 1, LaneSize - 1);
        float w = pos - static_cast<float>(i1);

        float v1 = static_cast<float>(rawValues[i1]) / 255.0f;
        float v2 = static_cast<float>(rawValues[i2]) / 255.0f;
        return v1 + w * (v2 - v1);
    }

    // --- Swap queued into active ------------------------------------------

    void swapQueued() {
        constexpr size_t groupSlice = MAX_VOICES * MAX_PAGES;
        std::memcpy(&sequencerData.allGroupPages[0],
                    &sequencerData.queued[0],
                    groupSlice * sizeof(PageOfStepsCpp));
        sequencerData.allGroupDurations[0] = sequencerData.queuedGroupDurations[0];
        std::memcpy(&sequencerData.allGroupPageIds[0],
                    &sequencerData.queuedGroupPageIds[0],
                    groupSlice * sizeof(int));
        std::memcpy(&sequencerData.allGroupPatternIds[0],
                    &sequencerData.queuedGroupPatternIds[0],
                    groupSlice * sizeof(UUIDKey));

        std::memcpy(&allGroupPages[0],
                    &sequencerData.queued[0],
                    groupSlice * sizeof(PageOfStepsCpp));
        std::memcpy(&allGroupPageIds[0],
                    &sequencerData.queuedGroupPageIds[0],
                    groupSlice * sizeof(int));
        std::memcpy(&allGroupPatternIds[0],
                    &sequencerData.queuedGroupPatternIds[0],
                    groupSlice * sizeof(UUIDKey));

        sequencerData.hasQueued = false;
    }

    // --- configureSong ----------------------------------------------------

    void configureSong() {
        std::memcpy(&ratchetDurations, &sequencerData.ratchetDurations, sizeof(ratchetDurations));
        stepDuration = sequencerData.stepDuration;
        std::memcpy(&stepDurations, &sequencerData.stepDurations, sizeof(stepDurations));

        std::memcpy(&allGroupPages, &sequencerData.allGroupPages, sizeof(allGroupPages));
        std::memcpy(&allGroupPatternIds, &sequencerData.allGroupPatternIds, sizeof(allGroupPatternIds));
        std::memcpy(&allGroupPageIds, &sequencerData.allGroupPageIds, sizeof(allGroupPageIds));

        loopedGroupIndex = std::nullopt;

        if (sequencerData.hasLoopPattern) {
            loopPatternFunc(
                sequencerData.loopPatternSolo,
                sequencerData.loopPattern,
                sequencerData.loopPatternData
            );
            return;
        }

        if (sequencerData.hasCurrentlySelectedLoopGroup) {
            loopedGroupIndex = static_cast<int>(sequencerData.currentlySelectedLoopGroup);
        }

        useGroup();
    }

    // --- updatePosition ---------------------------------------------------

    void updatePosition() {
        Position& position = PositionContainerWrapper::instance.getWriteable();

        if (sequencerData.isPerforming) {
            std::memset(&position.loopGroup, 0, sizeof(UUIDKey));
        } else if (sequencerData.hasLoopPattern) {
            if (sequencerData.hasCurrentlySelectedLoopGroup && !sequencerData.loopPatternSolo) {
                const int groupIdx = static_cast<int>(sequencerData.currentlySelectedLoopGroup);
                std::memcpy(&position.loopGroup,
                            &sequencerData.loopGroups[groupIdx].id,
                            sizeof(UUIDKey));
            } else {
                std::memset(&position.loopGroup, 0, sizeof(UUIDKey));
            }
        } else {
            std::memcpy(&position.loopGroup,
                        &sequencerData.loopGroups[currentGroup].id,
                        sizeof(UUIDKey));
        }

        std::memcpy(&currentLoopGroup,
                    &position.loopGroup,
                    sizeof(UUIDKey));

        position.loopCount = currentGroupLoops;
        position.segment   = currentSegment;

        if (sequencerData.isPerforming || sequencerData.hasLoopPattern) {
            std::memset(&position.segmentId, 0, sizeof(UUIDKey));
        } else {
            std::memcpy(&position.segmentId,
                        &sequencerData.loopGroups[currentGroup].sections[currentSegment].id,
                        sizeof(UUIDKey));
        }

        position.step = currentStepWithinSegment;

        for (int v = 0; v < voiceCount; ++v) {
            const int idx = flatIndex(currentGroup, v, currentSegment);
            std::memcpy(&position.patterns[v], &allGroupPatternIds[idx], sizeof(UUIDKey));
            position.pageIds[v] = allGroupPageIds[idx];
        }

        PositionContainerWrapper::instance.publish();
    }

    // --- useGroup ---------------------------------------------------------

    void useGroup() {
        if (sequencerData.isPerforming || currentGroup >= sequencerData.loopGroupCount) {
            currentGroup = 0;
        }

        const int dur = sequencerData.allGroupDurations[currentGroup];
        loopGroupSteps = dur * StepsPerPage;

        for (int v = 0; v < voiceCount; ++v) {
            updateVoiceSequencer(
                voices[v],
                &allGroupPages[flatIndex(currentGroup, v, 0)],
                dur
            );
        }
    }

    // --- updateVoiceSequencer ---------------------------------------------

    void updateVoiceSequencer(VoiceSequencerCpp& voice,
                              const PageOfStepsCpp* pages,
                              int pageCount) {
        voice.track.bpm   = static_cast<int32_t>(sequencerData.bpm);
        voice.track.swing = static_cast<int32_t>(sequencerData.swing);

        std::memcpy(&voice.track.ratchetDurations, ratchetDurations.data(), 4 * sizeof(int32_t));
        std::memcpy(&voice.track.pages, pages, pageCount * sizeof(PageOfStepsCpp));

        voice.track.numberOfPages = static_cast<int32_t>(pageCount);
    }

    // --- loopPatternFunc --------------------------------------------------

    void loopPatternFunc(bool solo,
                         const UUIDKey& patternId,
                         const PatternLaneConfigCpp& lane) {

        // Case 1: merge into current loop group
        if (sequencerData.hasCurrentlySelectedLoopGroup && !solo) {
            const int currentGroupIdx = static_cast<int>(sequencerData.currentlySelectedLoopGroup);
            const SongLoopGroupCpp& groupToMerge = sequencerData.loopGroups[currentGroupIdx];

            bool containsPattern = false;
            for (int i = 0; i < groupToMerge.sectionsCount; ++i) {
                const auto& section = groupToMerge.sections[i];
                for (int j = 0; j < section.patternsCount; ++j) {
                    if (patternId == section.patterns[j]) {
                        containsPattern = true;
                        break;
                    }
                }
                if (containsPattern) break;
            }

            if (containsPattern) {
                std::memcpy(&allGroupPages, &sequencerData.allGroupPages, sizeof(allGroupPages));
                std::memcpy(&allGroupPageIds, &sequencerData.allGroupPageIds, sizeof(allGroupPageIds));
                std::memcpy(&allGroupPatternIds, &sequencerData.allGroupPatternIds, sizeof(allGroupPatternIds));
                useGroup();
                return;
            }

            const int sectionCount = groupToMerge.sectionsCount;
            loopGroupSteps = std::max(sectionCount, static_cast<int>(lane.activePageCount)) * StepsPerPage;

            int emptyVoice = voiceCount - 1;
            for (int v = 0; v < voiceCount; ++v) {
                bool allEmpty = true;
                for (int i = 0; i < sectionCount; ++i) {
                    if (groupToMerge.sections[i].patterns[v] != EmptyUUID) {
                        allEmpty = false;
                        break;
                    }
                }
                if (allEmpty) { emptyVoice = v; break; }
            }

            updateVoiceSequencer(voices[emptyVoice], lane.pages.data(), lane.activePageCount);

            for (int page = 0; page < (loopGroupSteps / StepsPerPage); ++page) {
                const int slot = page % sectionCount;
                const int idx  = flatIndex(currentGroupIdx, emptyVoice, slot);
                std::memcpy(&allGroupPatternIds[idx], &patternId, sizeof(UUIDKey));
                allGroupPageIds[idx] = page % static_cast<int>(lane.activePageCount);
            }

            return;
        }

        // Case 2: performing mode
        if (sequencerData.isPerforming) {
            const int hardcodedPatternVoice = voiceCount - 1;

            int currentLoopGroupSteps = loopGroupSteps;
            loopGroupSteps = std::max(loopGroupSteps, static_cast<int>(lane.activePageCount) * StepsPerPage);

            // BUG: this loop doubles `currentLoopGroupSteps` (a local) until it reaches
            // `loopGroupSteps`, but the result is never written back — dead code.
            // Likely intended to snap loopGroupSteps to a power-of-two boundary.
            while (currentLoopGroupSteps < loopGroupSteps) {
                currentLoopGroupSteps *= 2;
            }

            updateVoiceSequencer(voices[hardcodedPatternVoice], lane.pages.data(), lane.activePageCount);

            for (int page = 0; page < lane.activePageCount; ++page) {
                const int idx = flatIndex(currentGroup, hardcodedPatternVoice, page);
                std::memcpy(&allGroupPatternIds[idx], &patternId, sizeof(UUIDKey));
                allGroupPageIds[idx] = page;
            }

            return;
        }

        // Case 3: normal solo loop
        currentGroup = 0;
        loopGroupSteps = static_cast<int>(lane.activePageCount) * StepsPerPage;

        const int hardcodedPatternVoice = voiceCount - 1;

        updateVoiceSequencer(voices[hardcodedPatternVoice], lane.pages.data(), lane.activePageCount);

        for (int page = 0; page < lane.activePageCount; ++page) {
            const int idx = flatIndex(currentGroup, hardcodedPatternVoice, page);
            std::memcpy(&allGroupPatternIds[idx], &patternId, sizeof(UUIDKey));
            allGroupPageIds[idx] = page;
        }

        for (int v = 0; v < hardcodedPatternVoice; ++v) {
            updateVoiceSequencer(voices[v], cleanPages.data(), 1);

            for (int page = 0; page < lane.activePageCount; ++page) {
                const int idx = flatIndex(currentGroup, v, page);
                allGroupPatternIds[idx] = EmptyUUID;
                allGroupPageIds[idx] = 0;
            }
        }
    }
};
