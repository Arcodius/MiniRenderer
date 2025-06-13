#pragma once

#include <string>

#include "Material.h"
#include "Mesh.h"


struct Intersection;
class Material;
struct Ray;

class Object {
public:
	enum PrimitiveType {
		PLANE,
		CUBE,
		SPHERE,
		CYLINDER,
		CONE,
		TORUS
	};
protected:
	Mesh mesh;
	bool isPrimitive = false;
	Object::PrimitiveType primitive;
	std::shared_ptr<Material> material;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::mat4 matrix;

	glm::vec3 delta_position;
	glm::vec3 delta_rotation;
	glm::vec3 delta_scale;

public:
	Object() : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1) {
		matrix = glm::mat4();
	}
	Object(const Mesh& mesh, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
		: mesh(mesh), position(position), rotation(rotation), scale(scale) {
		update();
	}

	void update();
	void translate(const glm::vec3& translation);
	void rotate(const glm::vec3& rotationDelta);
	void resize(const glm::vec3& scaleDelta);
	void setPosition(const glm::vec3& newPosition);
	void setRotation(const glm::vec3& newRotation);
	void setScale(const glm::vec3& newScale);
    std::string getName() const { return mesh.getName(); }
	glm::vec3 getPosition() const { return position; }
	glm::vec3 getRotation() const { return rotation; }
	glm::vec3 getScale() const { return scale; }
	glm::mat4 getMatrix() const { return matrix; }

	void setMesh(const std::string& meshPath);
	Mesh& getMesh() { return mesh; }

	void setAsPrimitive(Object::PrimitiveType PrimitiveType);

	bool hasMaterial() const { return material != nullptr; }
	std::shared_ptr<Material> getMaterial() const { return material; }
    void setMaterial(const std::shared_ptr<Material>& m) { material = m; }
};

class Sphere : public Object {
private:
    float radius; // Sphere radius
    
public:
    Sphere(const glm::vec3& position, float r, const Material& m)
        : Object(Mesh(), position, glm::vec3(0.0f), glm::vec3(1.0f)), radius(r) {
        setAsPrimitive(Object::PrimitiveType::SPHERE); // Mark as primitive
    }

    float getRadius() const { return radius; }
    void setRadius(float r) { radius = r; }

    

    // Sphere intersection logic
    bool intersect(const Ray& ray, Intersection& isect) const;
};