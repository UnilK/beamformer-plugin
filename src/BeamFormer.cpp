#include "BeamFormer.h"

#include "constants.h"

#include <complex>
#include <cmath>
#include <iostream>

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
        f *= 1.0f + (0.25f / relativeHalfTime);
    }
    return {freq, decay};
}

std::vector<float> beamform(
    const FilterState& fstate,
    const std::vector<vec3>& micPositions,
    std::vector<vec3> directions)
{
    int dirs = (int)directions.size();
    std::vector<float> energyPattern(dirs, 0.0f);

    int filters = (int)fstate.averageAngularVelocity.size();
    if(filters == 0) return energyPattern;
    int mics = (int)fstate.averageFilterPhases[0].size();
    if((int)micPositions.size() != mics) return energyPattern;
    
    // normalize directions
    for(vec3 &v : directions) v = v / v.abs();

    // compute wave numbers of frequencies detected in filters
    std::vector<float> waveNumber(filters);
    for(int i=0; i<filters; i++){
        if(std::norm(fstate.averageAngularVelocity[i]) == 0) continue;
        waveNumber[i] = std::arg(fstate.averageAngularVelocity[i]) * fstate.fs / fstate.c;
    }

    std::vector<std::complex<float> > energyLayer(dirs);
    for(int filter=0; filter<filters; filter++){
        std::fill(energyLayer.begin(), energyLayer.end(), 0.0f);

        // Beamformer!
        for(int mic=0; mic<mics; mic++){
            for(int dir=0; dir<dirs; dir++){
                energyLayer[dir] += fstate.averageFilterPhases[filter][mic]
                    * std::polar(1.0f, waveNumber[filter] * directions[dir].dotyz(micPositions[mic]));
            }
        }

        // visualize by mapping the energy range to [0, nmax]. add to total sum
        float nmin = 1e18f;
        float nmax = -1.0f;
        for(auto &i : energyLayer){
            nmin = std::min(nmin, std::norm(i));
            nmax = std::max(nmax, std::norm(i));
        }

        if(nmin >= nmax) continue;

        float irange = nmax / (nmax - nmin);
        for(int i=0; i<dirs; i++){
            energyPattern[i] += (std::norm(energyLayer[i])-nmin) * irange;
        }
    }

    float nmin = 1e18f;
    float nmax = -1.0f;
    for(auto &i : energyPattern){
        nmin = std::min(nmin, i);
        nmax = std::max(nmax, i);
    }

    if(nmin >= nmax) return energyPattern;

    float irange = 1.0f / (nmax - nmin);
    for(int i=0; i<dirs; i++){
        energyPattern[i] = (energyPattern[i]-nmin) * irange;
    }

    return energyPattern;
}