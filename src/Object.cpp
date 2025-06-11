#include "Object.h"

#include <SDL3/SDL_log.h>
#include "ResourceLoader.h"
#include "MyMath.h"

void Object::update() {
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0, 0, 1)) *
                               glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0, 1, 0)) *
                               glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
    matrix = translationMatrix * rotationMatrix * scaleMatrix;
}


void Object::translate(const glm::vec3& translation) {
	delta_position = translation;
	update();
}

void Object::rotate(const glm::vec3& rotationDelta) {
	delta_rotation = rotationDelta;
	update();
}

void Object::resize(const glm::vec3& scaleDelta) {
	delta_scale = scaleDelta;
	update();
}

void Object::setPosition(const glm::vec3& newPosition) {
	position = newPosition;
	update();
}

void Object::setRotation(const glm::vec3& newRotation) {
	rotation = newRotation;
	update();
}

void Object::setScale(const glm::vec3& newScale) {
	scale = newScale;
	update();
}

void Object::setMesh(const std::string& meshPath) {
	Mesh new_mesh;
	bool success = ResourceLoader::loadMeshFromFile(meshPath, new_mesh);
	if (!success) {
        SDL_Log("Failed to load mesh from file: %s", meshPath.c_str());
		return;
	}
	mesh = new_mesh;
}