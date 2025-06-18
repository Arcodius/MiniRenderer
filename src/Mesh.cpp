#include "Mesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <iostream>

void Mesh::clear() {
    vertices.clear();
    indices.clear();
}

bool Mesh::intersect(const Ray& ray, Intersection& isect, const std::shared_ptr<Material>& material) const {
    bool hit = false;
    for (const auto& triangle : triangles) {
        if (triangle.intersect(ray, isect, material)) {
            hit = true;
        }
    }
    return hit;
}