#include "Mesh.h"

#include "MyMath.h"
#include "ResourceLoader.h"

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

Mesh::Mesh() : name("default") {}

Mesh::Mesh(const std::string& path) : name(path) {
    ResourceLoader::loadMeshFromFile(path, *this);
}
