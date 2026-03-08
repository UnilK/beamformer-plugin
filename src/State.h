#pragma once

#include "MathUtil.h"

#include <vector>
#include <complex>

struct State {
    vec3 targetPosition{10, 0, 0};
    vec3 outTargetPosition{10, 0, 0};
    std::vector<vec3> micPositions;

    float mind = 1e-3f;
    float maxd = 100;

    float fps = 30.0f;
    float frameDecayRate = 0.1f;

    bool targetNoise = true;
    bool targetSine = false;
    float targetFrequency = 5000;
    float outVolumedB = -10.0f;
    float nsr = 0.0f;

    bool disableFrequencyTracking = false;
    bool disablePhaseAveraging = false;
};

struct FilterState {
    float fs = 48000;
    float c = 343;
    std::vector<float> micAngularFrequencies;
    std::vector<float> micRelativeDecays;
    std::vector<std::vector<std::complex<float>>> currentFilterPhases;
    std::vector<std::vector<std::complex<float>>> averageFilterPhases;
    std::vector<std::complex<float> > averageAngularVelocity;
};
