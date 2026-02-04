#pragma once

#include "MathUtil.h"

#include <vector>
#include <complex>

struct State {
    vec3 targetPosition{1, 0, 0};
    std::vector<float> micAngularFrequencies;
    std::vector<float> micRelativeDecays;
    std::vector<vec3> micPositions;
    int targetSoundType = 0;
    float targetStrength = 1;
    int noiseType = 0;
    float noiseStrength = 0;

    float c = 343;
    float mind = 1e-3f;
    float maxd = 100;
};

struct MicFilters {
    std::vector<std::complex<float> > micPhases;
    std::vector<std::complex<float> > micFrequencies;
};
