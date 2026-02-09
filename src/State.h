#pragma once

#include "MathUtil.h"

#include <vector>
#include <complex>

struct State {
    vec3 targetPosition{10, 0, 0};
    std::vector<vec3> micPositions;
    int targetSoundType = 0;
    float targetStrength = 1;
    int noiseType = 0;
    float noiseStrength = 0;

    float mind = 1e-3f;
    float maxd = 100;

    float fps = 30.0f;
    float frameDecayRate = 0.3f;

    bool targetNoise = true;
    bool targetSine = false;
    float targetFrequency = 5000;
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
