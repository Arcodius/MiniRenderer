#pragma once
#include "Color.h"
#include "Math.h"

class Light {
public:
	Color color;
	float intensity;

	Light() : color(Color()), intensity(1.0f) {}
	Light(const Color& color_, float intensity_) : color(color_), intensity(intensity_) {}
    virtual ~Light() = default;

	Color getColor() const { return color; }
	virtual Vec3 getDirection(const Vec3& point) const = 0;
	virtual float getIntensity(const Vec3& point) const = 0;
	virtual float getDistance(const Vec3& point) const = 0;
};

class DirectionalLight : public Light {
public:
	Vec3 direction;

	DirectionalLight() : Light(), direction(Vec3(0.0f, -1.0f, 0.0f)) {}
	DirectionalLight(const Color& color_, float intensity_, const Vec3& direction_)
		: Light(color_, intensity_), direction(direction_) {}

	Vec3 getDirection(const Vec3& point) const override {
		return direction.normalized();
	}

	float getIntensity(const Vec3& point) const override {
		return intensity;
	}

    float getDistance(const Vec3& point) const override {
		return 0.0f; // parallel light has no distance
	}
};

class PointLight : public Light {
public:
	Vec3 position;
	float range;

	PointLight() : Light(), position(Vec3(0.0f, 0.0f, 0.0f)), range(1.0f) {}
	PointLight(const Color& color_, float intensity_, const Vec3& position_, float range_)
		: Light(color_, intensity_), position(position_), range(range_) {}

	Vec3 getDirection(const Vec3& point) const override {
		return (point - position).normalized();
	}

	float getIntensity(const Vec3& point) const override {
		float distance = getDistance(point);
		//if (distance > range) return 0;
		return intensity / (distance * distance);
	}

    float getDistance(const Vec3& point) const override {
        return (point - position).length();
    }
};

class SpotLight : public Light {
public:
    Vec3 position;      
    Vec3 direction;     
    float range;        
    float innerAngle;   
    float outerAngle;   
    float falloff;      

    SpotLight()
        : Light(),
        position(Vec3(0.0f)),
        direction(Vec3(0.0f, -1.0f, 0.0f)),
        range(10.0f),
        innerAngle(30.0f),
        outerAngle(45.0f),
        falloff(1.0f) {}

    SpotLight(const Color& color, float intensity,
        const Vec3& position, const Vec3& direction,
        float range, float innerAngle, float outerAngle,
        float falloff = 1.0f)
        : Light(color, intensity),
        position(position),
        direction(direction.normalized()),
        range(range),
        innerAngle(innerAngle),
        outerAngle(outerAngle),
        falloff(falloff) {}


    Vec3 getDirection(const Vec3& point) const override {
        return (position - point).normalized();
    }


    float getIntensity(const Vec3& point) const override {

        float distance = (position - point).length();
        if (distance > range) return 0.0f;

        float distanceAttenuation = 1.0f - (distance / range);

        Vec3 lightDir = getDirection(point);
        float cosTheta = direction.dot(lightDir);


        float cosInner = std::cos(innerAngle * 3.14159265f / 180.0f);
        float cosOuter = std::cos(outerAngle * 3.14159265f / 180.0f);


        float spotEffect = 0.0f;
        if (cosTheta > cosInner) {
            spotEffect = 1.0f;
        }
        else if (cosTheta > cosOuter) {

            spotEffect = std::pow((cosTheta - cosOuter) / (cosInner - cosOuter), falloff);
        }

        float totalAttenuation = distanceAttenuation * spotEffect;
        return  intensity * totalAttenuation;
    }

    float getDistance(const Vec3& point) const override {
        return (position - point).length();
    }

    void setDirection(const Vec3& newDirection) {
        direction = newDirection.normalized();
    }

    void setAngles(float newInnerAngle, float newOuterAngle) {
        innerAngle = (std::min)(newInnerAngle, newOuterAngle);
        outerAngle = (std::max)(newInnerAngle, newOuterAngle);
    }
};