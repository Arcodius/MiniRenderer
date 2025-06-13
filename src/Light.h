#pragma once

#include "MyMath.h"

class Light {
public:
	glm::vec3 color;
	float intensity;

	Light() : color(glm::vec3(1.0f)), intensity(1.0f) {}
	Light(const glm::vec3& color_, float intensity_) : color(color_), intensity(intensity_) {}
    virtual ~Light() = default;

	glm::vec3 getColor() const { return color; }
	virtual glm::vec3 getDirection(const glm::vec3& point) const = 0;
	virtual float getIntensity(const glm::vec3& point) const = 0;
	virtual float getDistance(const glm::vec3& point) const = 0;
};

class DirectionalLight : public Light {
public:
	glm::vec3 direction;

	DirectionalLight() : Light(), direction(glm::vec3(0.0f, -1.0f, 0.0f)) {}
	DirectionalLight(const glm::vec3& color_, float intensity_, const glm::vec3& direction_)
		: Light(color_, intensity_), direction(direction_) {}

	glm::vec3 getDirection(const glm::vec3& point) const override {
		return glm::normalize(direction); // Directional light direction is constant
	}

	float getIntensity(const glm::vec3& point) const override {
		return intensity;
	}

    float getDistance(const glm::vec3& point) const override {
		return 0.0f; // parallel light has no distance
	}
};

class PointLight : public Light {
public:
	glm::vec3 position;
	float range;

	PointLight() : Light(), position(glm::vec3(0.0f, 0.0f, 0.0f)), range(1.0f) {}
	PointLight(const glm::vec3& color_, float intensity_, const glm::vec3& position_, float range_)
		: Light(color_, intensity_), position(position_), range(range_) {}

    glm::vec3 getPosition() const {
        return position;
    }

	glm::vec3 getDirection(const glm::vec3& point) const override {
		return glm::normalize(position - point);
	}

	float getIntensity(const glm::vec3& point) const override {
        float distance2 = static_cast<float>((point - position).length());
        return intensity / (std::pow(distance2, 2.f) + EPSILON); // 或使用 (a + b*d + c*d^2)
    }

    float getDistance(const glm::vec3& point) const override {
        return static_cast<float>((point - position).length());
    }
};

class SpotLight : public Light {
public:
    glm::vec3 position;      
    glm::vec3 direction;     
    float range;        
    float innerAngle;   
    float outerAngle;   
    float falloff;      

    SpotLight()
        : Light(),
        position(glm::vec3(0.0f)),
        direction(glm::vec3(0.0f, -1.0f, 0.0f)),
        range(10.0f),
        innerAngle(30.0f),
        outerAngle(45.0f),
        falloff(1.0f) {}

    SpotLight(const glm::vec3& color, float intensity,
        const glm::vec3& position, const glm::vec3& direction,
        float range, float innerAngle, float outerAngle,
        float falloff = 1.0f)
        : Light(color, intensity),
        position(position),
        direction(glm::normalize(direction)),
        range(range),
        innerAngle(innerAngle),
        outerAngle(outerAngle),
        falloff(falloff) {}
    
    glm::vec3 getPosition() const {
        return position;
    }

    glm::vec3 getDirection(const glm::vec3& point) const override {
        return glm::normalize(position - point);
    }


    float getIntensity(const glm::vec3& point) const override {

        float distance = static_cast<float>((position - point).length());
        if (distance > range) return 0.0f;

        float distanceAttenuation = 1.0f - (distance / range);

        glm::vec3 lightDir = getDirection(point);
        float cosTheta = glm::dot(lightDir, direction);


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

    float getDistance(const glm::vec3& point) const override {
        return static_cast<float>((position - point).length());
    }

    void setDirection(const glm::vec3& newDirection) {
        direction = glm::normalize(newDirection);
    }

    void setAngles(float newInnerAngle, float newOuterAngle) {
        innerAngle = (std::min)(newInnerAngle, newOuterAngle);
        outerAngle = (std::max)(newInnerAngle, newOuterAngle);
    }
};