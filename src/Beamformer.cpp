#include "Beamformer.h"

#include "constants.h"

#include <complex>
#include <cmath>

std::vector<vec3> createShowerFlowerArray(int arms, int armlength, float curve, float radius){
    std::vector<std::complex<float>> petal;
    for(int i=0; i<armlength; i++){
        petal.push_back(std::polar<float>((float)std::sqrt(1+i) - 0.6f, curve * i));
    }

    float maxd = 1e-18f;
    for(auto &i : petal) maxd = std::max(maxd, std::abs(i));
    for(auto &i : petal) i *= radius / maxd;

    std::vector<vec3> flower;
    for(int i=0; i<arms; i++){
        for(auto &p : petal){
            auto c = std::polar(1.0f, 2 * PIF * i / arms) * p;
            flower.emplace_back(0, c.real(), c.imag());
        }
    }

    return flower;
}

std::tuple<std::vector<float>, std::vector<float>> createFilterBank(float fs, float firstFreq, float relativeHalfTime){
    float f = firstFreq;
    std::vector<float> freq;
    std::vector<float> decay;
    while(f < 0.9f * fs * 0.5f){
        freq.push_back(2.0f*PIF*f/fs);
        decay.push_back(relativeHalfTime);
        f *= 1 + (0.25 / relativeHalfTime);
    }
    return {freq, decay};
}

std::vector<float> beamform(
    const FilterState& fstate,
    const std::vector<vec3>& micPositions,
    const std::vector<vec3>& directions,
    const float& fs,
    const float& c)
{
    int dirs = directions.size();
    std::vector<float> energyPattern(dirs);

    int filters = fstate.averageAngularVelocity.size();
    if(filters == 0) return energyPattern;
    int mics = fstate.averageFilterPhases[0].size();

    std::vector<float> waveNumber(filters);
    for(int i=0; i<filters; i++){
        if(std::norm(fstate.averageAngularVelocity[i]) == 0) continue;
        waveNumber[i] = std::arg(fstate.averageAngularVelocity[i]) * fs / c;
    }
}