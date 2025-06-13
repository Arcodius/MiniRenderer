#pragma once

#include <memory>

#include "Material.h"
#include "MyMath.h"

struct Intersection {
    float t;
    glm::vec3 position;
    glm::vec3 normal;
    std::shared_ptr<Material> material;
    bool hit;

    Intersection() : t(std::numeric_limits<float>::max()), hit(false) {}
};