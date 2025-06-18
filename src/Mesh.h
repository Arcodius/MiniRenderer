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

    size_t firstTriangleIdx = -1;
    size_t numTriangles = 0;

    Mesh();
    Mesh(const std::string& path);
    
    std::string getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
	void clear();

    void output() const;

    // 逐三角形求交
    bool intersect(const Ray& ray, Intersection& isect, const std::shared_ptr<Material>& material) const;
    // BVH求交
    bool intersect(const Ray& ray, Intersection& isect) const;
};