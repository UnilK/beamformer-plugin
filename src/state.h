#include <vector>

struct vec3 {
    float x = 0, y = 0, z = 0;
};

struct state {
    vec3 targetPosition;
    std::vector<vec3> micPositions;
    int targetSoundType = 0;
    float targetStrength = 1;
    int noiseType = 0;
    float noiseStrength = 0;
};