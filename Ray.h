#pragma once

#include "Math.h"

class Ray {
public:
	Vec3 o; // The starting point of the ray
	Vec3 t; // The direction in which the ray is pointing

	// Constructor to initialize a ray with an origin and direction
	Ray(const Vec3& origin, const Vec3& direction)
		: o(origin), t(direction.normalized()) {}

	// Method to get a point along the ray at a given distance t
	Vec3 at(float scalar) const {
		return o + t * scalar;
	}
};