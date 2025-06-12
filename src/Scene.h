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
public:
	std::vector<Object> objects;
	std::vector<std::shared_ptr< Light >> lights;

	Camera camera;

	Scene() : camera() {
		// TODO: load from config
		Object plane;
		plane.setMesh("Resources\\plane.obj");
		plane.setPosition(glm::vec3(0, 0, 0.0f));
		plane.setRotation(glm::vec3(0, 0, 0));
		plane.setScale(glm::vec3(3.f));
		addObject(plane);

		Object cube;
		cube.setMesh("Resources\\cube.obj");
		cube.setPosition(glm::vec3(0, 0.5f, 0.0f));
		cube.setRotation(glm::vec3(0));
		cube.setScale(glm::vec3(0.5f));
		addObject(cube);

		addLight(std::make_shared<PointLight>(
			Color(glm::vec3(1.0f, 1.0f, 0.0f)), 10.0f, glm::vec3(0.0f, 4.0f, 0.0f), 10.0f
		));

		camera.setPerspective(true);
		camera.setFovY(120.f);

        camera.setPosition(glm::vec3(2.0f));
        camera.setTarget(glm::vec3(0, 0, 2.0f));
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