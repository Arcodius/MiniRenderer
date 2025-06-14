#pragma once
#include <string>
#include <vector>
#include "Vertex.h"
#include "Triangle.h"

class Mesh {
private:
    std::string name;
public:
    
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
    std::vector<Triangle> triangles;

    std::string getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
    void applyTransform(const glm::mat4& transform);
	void clear();

    void output() const {
        printf("Mesh: %s\n", name.c_str());
        printf("Vertices: %zu, Indices: %zu\n", vertices.size(), indices.size());
        for (const auto& vertex : vertices) {
            printf("Vertex: pos=(%.2f, %.2f, %.2f), normal=(%.2f, %.2f, %.2f), uv=(%.2f, %.2f)\n",
                   vertex.worldPos.x, vertex.worldPos.y, vertex.worldPos.z,
                   vertex.normal.x, vertex.normal.y, vertex.normal.z,
                   vertex.uv.x, vertex.uv.y);
        }
    }

    bool intersect(const Ray& ray, Intersection& isect, const std::shared_ptr<Material>& material) const;
};