#pragma once

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
