#pragma once

#include "MyMath.h"

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
    float t_min = EPSILON;
    float t_max = std::numeric_limits<float>::max();

    Ray(const glm::vec3& o, const glm::vec3& d)
        : origin(o), direction(glm::normalize(d)) {}
    Ray(const glm::vec3& o, const glm::vec3& d, float max_dist)
        : origin(o), direction(glm::normalize(d)), t_max(max_dist) {}
};
