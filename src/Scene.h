#pragma once
#include <iostream>
#include <memory>
#include <vector>

#include "BVH.h"
#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include "Object.h"

#include <iostream>

class Scene {
private:
	glm::vec3 backgroundColor = glm::vec3(0.05f);
	int MAX_PRIMS_IN_LEAF = 8;

    std::vector<BVHNode> bvhNodes;
    std::vector<Triangle> flattenedTriangles;
    std::vector<int> primitiveIndices;
    int rootNodeIdx = -1; // BVH 根节点的索引	

    // 递归构建 BVH 的辅助函数
    int buildBVHRecursive(std::vector<int>& primitiveIndices, int start, int end, int currentDepth);
    // 获取图元 (三角形) 的 AABB
    AABB getPrimitiveAABB(int primitiveIdx) const;
	
public:
	std::vector<std::shared_ptr< Object >> objects; // primitive objects use shared_ptr for polymorphism
	std::vector<std::shared_ptr< Light >> lights;

	Camera camera;

	Scene();

	void setup();

	void buildBVH();

	void addObject(const std::shared_ptr<Object>& object) {
		objects.push_back(object);
	}

	void removeObject(size_t index) {
		if (index < objects.size()) {
			objects.erase(objects.begin() + index);
		}
	}

	void addLight(const std::shared_ptr<Light>& light) {
		lights.emplace_back(light);
	}

	void removeLight(size_t index) {
		if (index < lights.size()) {
			lights.erase(lights.begin() + index);
		}
	}

	void clear() { 
		objects.clear();
		lights.clear();
	}

	glm::vec3 getBackgroundColor() const {
		return backgroundColor;
	}

	bool intersect(const Ray& ray, Intersection& closestIsect) const;

};

