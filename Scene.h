#pragma once
#include <vector>

#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include "Object.h"

class Scene {
public:
	std::vector<Object> objects;
	std::vector<std::shared_ptr< Light >> lights;

	Camera camera;

	Scene() : camera() {
		// TODO: load from config
		Object cone;
		cone.setMesh("Resources/cone.obj");
		cone.setPosition(Vec3(0, 0, 1.0f));
		cone.setRotation(Vec3(0, 0, 1.57f));
		cone.setScale(Vec3(0.5f, 0.5f, 0.5f));
		addObject(cone);

		addLight(std::make_shared<PointLight>(
			Color(Vec3(1.0f, 1.0f, 0.3f)), 10.0f, Vec3(0.0f, 0.0f, 2.0f), 5.0f
		));

		camera.setPerspective(true);
		camera.setAspect(720.0f / 480.0f);
		camera.setFovY(45.0f);
	}

	void addObject(const Object& object) {
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

};