#pragma once

#include <vector>
#include <tuple>

#include "MathUtil.h"

std::vector<vec3> createShowerFlowerArray(int arms, int armlength, float curve, float radius = 1.0f);
std::tuple<std::vector<float>, std::vector<float>> createFilterBank(float fs, float firstFreq, float relativeHalfTime);