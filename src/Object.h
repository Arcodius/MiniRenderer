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
    // update matrix, apply to vertices, create triangles with material
    void updateMesh(); 

	void setAsPrimitive(Object::PrimitiveType PrimitiveType);

	bool hasMaterial() const { return material != nullptr; }
	std::shared_ptr<Material> getMaterial() const { return material; }
    void setMaterial(const std::shared_ptr<Material>& m) { material = m; }

	virtual bool intersect(const Ray& ray, Intersection& isect) const {return false;}
};

class GenericObject : public Object {
public:
    GenericObject(const Mesh& mesh, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, 
                  const std::shared_ptr<Material>& mat)
        : Object(mesh, position, rotation, scale) {
        isPrimitive = false; // 标记为非原始物体
        material = mat;
        updateMesh(); 
    }
    
    // iterate through all triangles to look for intersection
    virtual bool intersect(const Ray& ray, Intersection& isect) const override;
};

class Sphere : public Object {
private:
    float radius; // Sphere radius
    
public:
    Sphere(const glm::vec3& position, float r, const Material& m)
		: Object(Mesh(), position, glm::vec3(0.0f), glm::vec3(1.0f)), radius(r) {
		setMaterial(std::make_shared<Material>(m));
        setAsPrimitive(Object::PrimitiveType::SPHERE); // Mark as primitive
        updateRadius();
        updateMesh();
    }

    float getRadius() const { return radius; }
    void setRadius(float r) { radius = r; updateRadius(); }
    void updateRadius();

    // Sphere intersection logic
    virtual bool intersect(const Ray& ray, Intersection& isect) const override;
};

// Plane class
class Plane : public Object {
glm::vec3 normal;
bool finite;
public:
    Plane(bool finite, const glm::vec3& position, const glm::vec3& normal, const Material& m)
        : Object(Mesh(), position, glm::vec3(0.0f), glm::vec3(1.0f)), finite(finite), normal(normal) {
        updateRotation();
        setMaterial(std::make_shared<Material>(m));
        setAsPrimitive(Object::PrimitiveType::PLANE); // Mark as primitive
        updateMesh();
    }
    void updateRotation();
    virtual bool intersect(const Ray& ray, Intersection& isect) const override;
};

// Cube class
class Cube : public Object {
public:
    Cube(const glm::vec3& position, const glm::vec3& scale, const Material& m)
        : Object(Mesh(), position, glm::vec3(0.0f), scale) {
        setMaterial(std::make_shared<Material>(m));
        setAsPrimitive(Object::PrimitiveType::CUBE); // Mark as primitive
        updateMesh();
    }
	
    virtual bool intersect(const Ray& ray, Intersection& isect) const override;
};

// Cylinder class
class Cylinder : public Object {
private:
    float radius;
    float height;

public:
    Cylinder(const glm::vec3& position, float r, float h, const Material& m)
        : Object(Mesh(), position, glm::vec3(0.0f), glm::vec3(1.0f)), radius(r), height(h) {
        setMaterial(std::make_shared<Material>(m));
        setAsPrimitive(Object::PrimitiveType::CYLINDER); // Mark as primitive
        updateParameters();
        updateMesh();
    }

    float getRadius() const { return radius; }
    float getHeight() const { return height; }
    void setRadius(float r) { radius = r; updateParameters(); }
    void setHeight(float h) { height = h; updateParameters(); }
    void updateParameters();

    virtual bool intersect(const Ray& ray, Intersection& isect) const override;
};

// Cone class
class Cone : public Object {
private:
    float radius;
    float height;

public:
    Cone(const glm::vec3& position, float r, float h, const Material& m)
        : Object(Mesh(), position, glm::vec3(0.0f), glm::vec3(1.0f)), radius(r), height(h) {
        setMaterial(std::make_shared<Material>(m));
        setAsPrimitive(Object::PrimitiveType::CONE); // Mark as primitive
        updateMesh();
    }

    float getRadius() const { return radius; }
    float getHeight() const { return height; }
    void setRadius(float r) { radius = r; }
    void setHeight(float h) { height = h; }

    virtual bool intersect(const Ray& ray, Intersection& isect) const override;
};

// Torus class
class Torus : public Object {
// only suppports XZ plane torus
private:
    float majorRadius; // Distance from center to the tube center
    float minorRadius; // Radius of the tube

public:
    Torus(const glm::vec3& position, float majorR, float minorR, const Material& m)
        : Object(Mesh(), position, glm::vec3(0.0f), glm::vec3(1.0f)), majorRadius(majorR), minorRadius(minorR) {
        setMaterial(std::make_shared<Material>(m));
        setAsPrimitive(Object::PrimitiveType::TORUS); // Mark as primitive
        updateMesh();
    }

    float getMajorRadius() const { return majorRadius; }
    float getMinorRadius() const { return minorRadius; }
    void setMajorRadius(float majorR) { majorRadius = majorR; }
    void setMinorRadius(float minorR) { minorRadius = minorR; }

    virtual bool intersect(const Ray& ray, Intersection& isect) const override;
};