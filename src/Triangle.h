#pragma once

#include <array>

#include "Intersection.h"
#include "Ray.h"
#include "Vertex.h"

struct TransformedVertex {
    glm::vec3 worldPos;
    glm::vec3 worldNormal;
    glm::vec2 uv;
    TransformedVertex(): worldPos(0.0f), worldNormal(0.0f, 0.0f, 1.0f), uv(0.0f, 0.0f) {}
    TransformedVertex(const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uvCoords)
        : worldPos(pos), worldNormal(normal), uv(uvCoords) {}
};

struct Triangle{
    std::array<TransformedVertex, 3> vertices;
    std::shared_ptr<Material> material;

    Triangle(const TransformedVertex& v0, const TransformedVertex& v1, const TransformedVertex& v2, 
             const std::shared_ptr<Material>& mat) {
        vertices[0] = v0;
        vertices[1] = v1;
        vertices[2] = v2;
        material = mat;
    }
    Triangle(): vertices{ TransformedVertex(), TransformedVertex(), TransformedVertex()}, material(Material::defualtMat()) {};

    // Möller-Trumbore algorithm
    bool intersect(const Ray& ray, Intersection& isect) const;
};