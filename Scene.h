#pragma once
#include <vector>

#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include "Object.h"

class Scene {
public:
	std::vector<Object> objects;
	std::vector<Light> lights;

	Camera camera;

	Scene() : camera() {}

	void addObject(const Object& object) {
		objects.push_back(object);
	}

	void removeObject(size_t index) {
		if (index < objects.size()) {
			objects.erase(objects.begin() + index);
		}
	}

	void clear() { objects.clear(); }

};