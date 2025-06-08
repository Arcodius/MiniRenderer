#pragma once

#include "Mesh.h"
#include <string>

class Object {
private:
	Mesh mesh;
	Vec3 position;
	Vec3 rotation;
	Vec3 scale;
	Mat4 matrix;

	Vec3 delta_position;
	Vec3 delta_rotation;
	Vec3 delta_scale;

public:
	Object() : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1) {
		matrix = Mat4();
	}
	Object(const Mesh& mesh, const Vec3& position, const Vec3& rotation, const Vec3& scale)
		: mesh(mesh), position(position), rotation(rotation), scale(scale) {
		update();
	}

	void update();
	void translate(const Vec3& translation);
	void rotate(const Vec3& rotationDelta);
	void resize(const Vec3& scaleDelta);
	void setPosition(const Vec3& newPosition);
	void setRotation(const Vec3& newRotation);
	void setScale(const Vec3& newScale);
    std::string getName() const { return mesh.getName(); }
	Vec3 getPosition() const { return position; }
	Vec3 getRotation() const { return rotation; }
	Vec3 getScale() const { return scale; }
	Mat4 getMatrix() const { return matrix; }

	void setMesh(const std::string& meshPath);
	Mesh& getMesh() { return mesh; }
};