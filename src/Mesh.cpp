#include "Mesh.h"

#include "MyMath.h"
#include "ResourceLoader.h"

void Mesh::clear() {
    vertices.clear();
    indices.clear();
}

// 逐三角形求交
bool Mesh::intersect(const Ray& ray, Intersection& isect, const std::shared_ptr<Material>& material) const {
    bool hit = false;
    for (const auto& triangle : triangles) {
        if (triangle.intersect(ray, isect)) { // triangle's material is set in object constructor
            hit = true;
        }
    }
    return hit;
}

bool Mesh::intersect(const Ray& ray, Intersection& isect) const {
    return false; // placeholder for now, as Scene BVH will handle it
}

Mesh::Mesh() : name("default") {}

Mesh::Mesh(const std::string& path) : name(path) {
    ResourceLoader::loadMeshFromFile(path, *this);
}

 void Mesh::output() const {
    printf("Mesh: %s\n", name.c_str());
    printf("Vertices: %zu, Indices: %zu\n", vertices.size(), indices.size());
    for (const auto& vertex : vertices) {
        printf("Vertex: pos=(%.2f, %.2f, %.2f), normal=(%.2f, %.2f, %.2f), uv=(%.2f, %.2f)\n",
            vertex.localPos.x, vertex.localPos.y, vertex.localPos.z,
            vertex.normal.x, vertex.normal.y, vertex.normal.z,
            vertex.uv.x, vertex.uv.y);
    }
}