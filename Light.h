#pragma once
#include "Color.h"
#include "Math.h"

class Light {
public:
	Color color;
	float intensity;

	Light() : color(Color()), intensity(1.0f) {}
	Light(const Color& color_, float intensity_) : color(color_), intensity(intensity_) {}
};

class DirectionalLight : public Light {
public:
	Vec3 direction;

	DirectionalLight() : Light(), direction(Vec3(0.0f, -1.0f, 0.0f)) {}
	DirectionalLight(const Color& color_, float intensity_, const Vec3& direction_)
		: Light(color_, intensity_), direction(direction_) {}
};

class PointLight : public Light {
public:
	Vec3 position;
	float range;

	PointLight() : Light(), position(Vec3(0.0f, 0.0f, 0.0f)), range(1.0f) {}
	PointLight(const Color& color_, float intensity_, const Vec3& position_, float range_)
		: Light(color_, intensity_), position(position_), range(range_) {}
};

class SpotLight : public Light {
public:
	Vec3 position;
	Vec3 direction;
	float range;
	float angle;

	SpotLight() : Light(), position(Vec3(0.0f, 0.0f, 0.0f)), direction(Vec3(0.0f, -1.0f, 0.0f)), range(1.0f), angle(45.0f) {}
	SpotLight(const Color& color_, float intensity_, const Vec3& position_, const Vec3& direction_, float range_, float angle_)
		: Light(color_, intensity_), position(position_), direction(direction_), range(range_), angle(angle_) {}
};