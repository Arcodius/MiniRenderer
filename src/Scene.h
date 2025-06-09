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
		Object cone;
		cone.setMesh("Resources\\cube.obj");
		cone.setPosition(glm::vec3(0, 0, 0.5f));
		cone.setRotation(glm::vec3(0));
		cone.setScale(glm::vec3(1.0f));
		addObject(cone);

        Object cube;
        cube.setMesh("Resources\\torus.obj");
        cube.setPosition(glm::vec3(0, 0, 2.0f));
        cube.setRotation(glm::vec3(0, 66, 0));
        cube.setScale(glm::vec3(0.5f));
        addObject(cube);

		addLight(std::make_shared<PointLight>(
			Color(glm::vec3(1.0f, 1.0f, 0.3f)), 10.0f, glm::vec3(0.0f, 0.0f, 2.0f), 5.0f
		));

		camera.setPerspective(true);
		camera.setFovY(90.0f);

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