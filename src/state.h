#pragma once

#include <vector>
#include <complex>
#include <cmath>

struct vec3 {
    float x = 0, y = 0, z = 0;

    vec3 operator-(const vec3& rhs){
        return {x-rhs.x, y-rhs.y, z-rhs.z};
    }

    vec3 operator+(const vec3& rhs){
        return {x+rhs.x, y+rhs.y, z+rhs.z};
    }

    vec3 operator*(const float& s){
        return {x*s, y*s, z*s};
    }

    float abs(){
        return std::sqrt(x*x+y*y+z*z);
    }
};

struct State {
    vec3 targetPosition{1, 0, 0};
    std::vector<float> micFrequencies;
    std::vector<float> micDecays;
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
