#pragma once
#include <memory>
#include <vector>
#include <SDL3/SDL.h>
#include <iostream>
#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include "Object.h"

#include <iostream>

class Scene {
private:
	glm::vec3 backgroundColor = glm::vec3(0.1f);
public:
	std::vector<std::shared_ptr< Object >> objects; // primitive objects use shared_ptr for polymorphism
	std::vector<std::shared_ptr< Light >> lights;

	Camera camera;

	Scene() : camera() {}

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

