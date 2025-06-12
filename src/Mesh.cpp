#include "Mesh.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <iostream>

void Mesh::clear() {
    vertices.clear();
    indices.clear();
}

void Mesh::applyTransform(const glm::mat4& transform) {
    // output(); // Debug: Print mesh info

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

    // std::cout << "Transform matrix:\n" << glm::to_string(transform) << std::endl;
    // std::cout << "Normal matrix:\n" << glm::to_string(normalMatrix) << std::endl;

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

        // printf("Transformed normal: (%.3f, %.3f, %.3f)\n", vertex.normal.x, vertex.normal.y, vertex.normal.z);
    }

    // exit(0); // Debug exit
}
