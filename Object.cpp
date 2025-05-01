#include "Object.h"

#include "ObjLoader.h"

void Object::update() {
	matrix = Mat4().scale(scale).rotate(rotation).translate(position);
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
	bool success = ObjLoader::LoadFromFile("Resources/cube.obj", new_mesh);
	if (!success) {
		return;
	}
	mesh = new_mesh;
}