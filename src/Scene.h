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
	glm::vec3 backgroundColor = glm::vec3(0.0f);
public:
	std::vector<Object> objects;
	std::vector<std::shared_ptr< Light >> lights;

	Camera camera;

	Scene() : camera() {
		Material defaultMaterial;
		defaultMaterial.diffuseColor = glm::vec3(1.0f, 0.5f, 0.5f); // Light red
		defaultMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f); // White specular
		defaultMaterial.roughness = 0.5f; // Medium roughness
		defaultMaterial.metallic = 0.0f; // Non-metallic

		// TODO: load from config
		Object plane;
		plane.setAsPrimitive(Object::PrimitiveType::PLANE);
		plane.setPosition(glm::vec3(0, 0, 0.0f));
		plane.setRotation(glm::vec3(0, 0, 0));
		plane.setScale(glm::vec3(1.f));
		plane.setMaterial(std::make_shared<Material>(defaultMaterial));
		addObject(plane);

		Object sphere;
		sphere.setAsPrimitive(Object::PrimitiveType::SPHERE);
		sphere.setPosition(glm::vec3(0, 0.5f, 0.0f));
		sphere.setRotation(glm::vec3(0, 0, 0));
		sphere.setScale(glm::vec3(0.5f));
		plane.setMaterial(std::make_shared<Material>(defaultMaterial));
		addObject(sphere);

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

	glm::vec3 getBackgroundColor() const {
		return backgroundColor;
	}

};