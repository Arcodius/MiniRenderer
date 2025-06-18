#include "Triangle.h"

bool Triangle::intersect(const Ray& ray, Intersection& isect, const std::shared_ptr<Material>& material) const {
    // Get the three vertices of the triangle
    const glm::vec3& v0 = vertices[0].worldPos;
    const glm::vec3& v1 = vertices[1].worldPos;
    const glm::vec3& v2 = vertices[2].worldPos;

    // Compute edge vectors
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;

    // Compute the intersection point between the ray direction and the triangle plane
    glm::vec3 h = glm::cross(ray.direction, edge2);
    float a = glm::dot(edge1, h);

    // If a is close to zero, the ray is parallel to the triangle
    if (std::abs(a) < EPSILON) {
        return false;
    }

    float f = 1.0f / a;
    glm::vec3 s = ray.origin - v0;
    float u = f * glm::dot(s, h);

    // Check if u is within the range [0, 1]
    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(ray.direction, q);

    // Check if v is within the range [0, 1] and u + v <= 1
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    // Compute the ray parameter t
    float t = f * glm::dot(edge2, q);

    // Check if t is positive (correct direction)
    if (t > EPSILON && t < isect.t) {
        isect.t = t;
        isect.position = ray.origin + t * ray.direction;
        isect.normal = glm::normalize(glm::cross(edge1, edge2));
        isect.material = material; // Use object's material
        isect.uv = (1 - u - v) * vertices[0].uv + u * vertices[1].uv + v * vertices[2].uv;
        isect.normal = glm::normalize((1 - u - v) * vertices[0].worldNormal + u * vertices[1].worldNormal + v * vertices[2].worldNormal);
        isect.hit = true;
        return true;
    }

    return false;
}