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

	Scene() : camera() {
		Material defaultMaterial;
		defaultMaterial.diffuseColor = glm::vec3(1.0f, 0.1f, 0.1f); // Light red
		defaultMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f); // White specular
		defaultMaterial.roughness = 0.5f; // Medium roughness
		defaultMaterial.metallic = 0.0f; // Non-metallic

		Material blueMaterial;
		blueMaterial.diffuseColor = glm::vec3(0.1f, 0.1f, 1.0f); // Light blue
		blueMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f); // White specular
		blueMaterial.roughness = 0.1f;
		blueMaterial.metallic = 1.0f;
	

		std::shared_ptr<Cube> plane = std::make_shared<Cube>(glm::vec3(0.0f), glm::vec3(3.0f, 1.0f, 1.0f), defaultMaterial);
		addObject(plane);
		
		std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(glm::vec3(1.5f, 0.5f, 1.0f), 0.5f, defaultMaterial);
		sphere->setRotation(glm::vec3(0, 0, 0));
		sphere->setScale(glm::vec3(0.5f));
		addObject(sphere);

		std::shared_ptr<Sphere> sphere2 = std::make_shared<Sphere>(glm::vec3(-2.0f, 0.5f, -1.0f), 0.5f, blueMaterial);
		sphere2->setRotation(glm::vec3(0, 0, 0));
		sphere2->setScale(glm::vec3(0.5f));
		addObject(sphere2);

		std::shared_ptr<Cube> cube = std::make_shared<Cube>(glm::vec3(1.0f, 0.5f, 0.0f), glm::vec3(1.0f), defaultMaterial);
		cube->setRotation(glm::vec3(0, 0, 0));
		addObject(cube);

		addLight(std::make_shared<PointLight>(
			glm::vec3(1.0f, 1.0f, 0.0f), 5.0f, glm::vec3(0.0f, 4.0f, 0.0f), 100.0f
		));

		camera.setPerspective(true);
		camera.setFovY(90.f);

        camera.setPosition(glm::vec3(2.0f));
        camera.setTarget(glm::vec3(0, 0, 2.0f));
	}

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