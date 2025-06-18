#include "Mesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <iostream>

void Mesh::clear() {
    vertices.clear();
    indices.clear();
}

void Mesh::applyTransform(const glm::mat4& transform) {
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

    for (Vertex& vertex : vertices) {
        glm::vec4 pos = transform * glm::vec4(vertex.localPos, 1.0f);
        vertex.worldPos = glm::vec3(pos);

        glm::vec3 transformedNormal = normalMatrix * vertex.normal;

        if (glm::any(glm::isnan(transformedNormal))) {
            printf("NaN detected in transformed normal!\n");
            transformedNormal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        float len = glm::length(transformedNormal);
        if (len > 1e-5f) {
            vertex.normal = glm::normalize(transformedNormal);
        } else {
            printf("Warning: Normal degenerated to zero vector.\n");
            printf("Orig normal: (%.3f, %.3f, %.3f)\n", vertex.normal.x, vertex.normal.y, vertex.normal.z);
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Fallback
        }
    }
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