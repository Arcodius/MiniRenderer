#pragma once

#include "Intersection.h"
#include "Ray.h"
#include "Vertex.h"

struct Triangle{
    Vertex vertices[3];

    Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
        vertices[0] = v0;
        vertices[1] = v1;
        vertices[2] = v2;
    }
    Triangle(): vertices{ Vertex(), Vertex(), Vertex() } {};

    // Möller-Trumbore algorithm
    bool intersect(const Ray& ray, Intersection& isect, const std::shared_ptr<Material>& material) const;
};