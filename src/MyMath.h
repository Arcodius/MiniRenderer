#pragma once
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi

#define EPSILON 1e-6f
#define FLT_MAX 3.402823466e+38F
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

inline glm::vec4 lerp(const glm::vec4& v1, const glm::vec4& v2, float t) {
    return v1 + t * (v2 - v1);
}

inline glm::vec3 lerp(const glm::vec3& v1, const glm::vec3& v2, float t) {
    return v1 + t * (v2 - v1);
}

inline glm::vec2 lerp(const glm::vec2& v1, const glm::vec2& v2, float t) {
    return v1 + t * (v2 - v1);
}

inline glm::vec3 perspectiveLerp(const glm::vec3& v1, const glm::vec3& v2, const float t, const glm::vec4& v1c, const glm::vec4& v2c)
{
    float correctLerp = t * v2c.w / ((1 - t) * v1c.w + t * v2c.w);
    return lerp(v1, v2, correctLerp);
}