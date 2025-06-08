#include "Object.h"

#include <SDL3/SDL_log.h>
#include "ObjLoader.h"

void Object::update() {
	matrix = Mat4();
    matrix = Mat4::translation(position) *
             Mat4::rotationX(rotation.x) *
             Mat4::rotationY(rotation.y) *
             Mat4::rotationZ(rotation.z) *
             Mat4::scale(scale);
}

void Object::translate(const Vec3& translation) {
	delta_position = translation;
	update();
}

void Object::rotate(const Vec3& rotationDelta) {
	delta_rotation = rotationDelta;
	update();
}

void Object::resize(const Vec3& scaleDelta) {
	delta_scale = scaleDelta;
	update();
}

void Object::setPosition(const Vec3& newPosition) {
	position = newPosition;
	update();
}

void Object::setRotation(const Vec3& newRotation) {
	rotation = newRotation;
	update();
}

void Object::setScale(const Vec3& newScale) {
	scale = newScale;
	update();
}

void Object::setMesh(const std::string& meshPath) {
	Mesh new_mesh;
	bool success = ObjLoader::LoadFromFile(meshPath, new_mesh);
	if (!success) {
        SDL_Log("Failed to load mesh from file: %s", meshPath.c_str());
		return;
	}
	mesh = new_mesh;
}