#include "Object.h"

#include <SDL3/SDL_log.h>

#include "MyMath.h"
#include "Ray.h"
#include "ResourceLoader.h"
#include "Intersection.h"

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
	update();
	mesh.applyTransform(matrix);
}

void Object::setAsPrimitive(Object::PrimitiveType PrimitiveType) {
	isPrimitive = true;
	primitive = PrimitiveType;
	// Set default mesh based on primitive type
	switch (PrimitiveType) {
		case Object::PLANE:
			setMesh("Resources\\plane.obj");
			break;
		case Object::CUBE:
			setMesh("Resources\\cube.obj");
			break;
		case Object::SPHERE:
			setMesh("Resources\\sphere.obj");
			break;
		case Object::CYLINDER:
			setMesh("Resources\\cylinder.obj");
			break;
		case Object::CONE:
			setMesh("Resources\\cone.obj");
			break;
		case Object::TORUS:
			setMesh("Resources\\torus.obj");
			break;
	}
}

bool Sphere::intersect(const Ray& ray, Intersection& isect) const {
        glm::vec3 oc = ray.origin - position; // Use Object's position
        float a = glm::dot(ray.direction, ray.direction);
        float b = 2.0f * glm::dot(oc, ray.direction);
        float c = glm::dot(oc, oc) - radius * radius;

        float discriminant = b * b - 4 * a * c;

        if (discriminant < 0.0f) {
            return false;
        }

        float sqrtDisc = std::sqrt(discriminant);
        float t1 = (-b - sqrtDisc) / (2.0f * a);
        float t2 = (-b + sqrtDisc) / (2.0f * a);

        float t = (t1 > 0.001f) ? t1 : ((t2 > 0.001f) ? t2 : -1.0f);
        if (t < 0.0f) {
            return false;
        }

        if (t < isect.t) {
            isect.t = t;
            isect.position = ray.origin + t * ray.direction;
            isect.normal = glm::normalize(isect.position - position); // Use Object's position
            isect.material = material; // Assign shared_ptr<Material>
            isect.hit = true;
            return true;
        }

        return false;
    }