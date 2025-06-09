#pragma once

#include "MyMath.h"

class Ray {
public:
	glm::vec3 o; // The starting point of the ray
	glm::vec3 t; // The direction in which the ray is pointing

	// Constructor to initialize a ray with an origin and direction
	Ray(const glm::vec3& origin, const glm::vec3& direction)
		: o(origin), t(glm::normalize(direction)) {}

	// Method to get a point along the ray at a given distance t
	glm::vec3 at(float scalar) const {
		return o + t * scalar;
	}
};