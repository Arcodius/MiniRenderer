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
		if (distance > range) return 0;
		return intensity / (distance * distance);
	}

    float getDistance(const Vec3& point) const override {
        return (point - position).length();
    }
};

class SpotLight : public Light {
public:
    Vec3 position;      // 光源位置
    Vec3 direction;     // 光源照射方向(应该归一化)
    float range;        // 光照有效范围
    float innerAngle;   // 内锥角(度数)
    float outerAngle;   // 外锥角(度数)
    float falloff;      // 边缘衰减系数(0-1)

    // 默认构造函数
    SpotLight()
        : Light(),
        position(Vec3(0.0f)),
        direction(Vec3(0.0f, -1.0f, 0.0f)),
        range(10.0f),
        innerAngle(30.0f),
        outerAngle(45.0f),
        falloff(1.0f) {}

    // 完整参数构造函数
    SpotLight(const Color& color, float intensity,
        const Vec3& position, const Vec3& direction,
        float range, float innerAngle, float outerAngle,
        float falloff = 1.0f)
        : Light(color, intensity),
        position(position),
        direction(direction.normalized()),  // 确保方向是单位向量
        range(range),
        innerAngle(innerAngle),
        outerAngle(outerAngle),
        falloff(falloff) {}

    // 获取从点到光源的方向
    Vec3 getDirection(const Vec3& point) const override {
        return (position - point).normalized();
    }

    // 获取在指定点的光照强度(考虑衰减和聚光灯效果)
    float getIntensity(const Vec3& point) const override {
        // 计算距离衰减
        float distance = (position - point).length();
        if (distance > range) return 0.0f; // 超出范围无光照

        float distanceAttenuation = 1.0f - (distance / range); // 线性衰减

        // 计算聚光灯锥角衰减
        Vec3 lightDir = getDirection(point);
        float cosTheta = direction.dot(lightDir);

        // 将角度转换为余弦值
        float cosInner = std::cos(innerAngle * 3.14159265f / 180.0f);
        float cosOuter = std::cos(outerAngle * 3.14159265f / 180.0f);

        // 计算聚光灯效果
        float spotEffect = 0.0f;
        if (cosTheta > cosInner) {
            spotEffect = 1.0f;  // 在内锥角内
        }
        else if (cosTheta > cosOuter) {
            // 在外锥角内，平滑过渡
            spotEffect = std::pow((cosTheta - cosOuter) / (cosInner - cosOuter), falloff);
        }

        // 组合所有衰减因素
        float totalAttenuation = distanceAttenuation * spotEffect;
        return  intensity * totalAttenuation;
    }

    // 获取光源到点的距离
    float getDistance(const Vec3& point) const override {
        return (position - point).length();
    }

    // 设置聚光灯方向(自动归一化)
    void setDirection(const Vec3& newDirection) {
        direction = newDirection.normalized();
    }

    // 设置角度范围(确保内角不大于外角)
    void setAngles(float newInnerAngle, float newOuterAngle) {
        innerAngle = (std::min)(newInnerAngle, newOuterAngle);
        outerAngle = (std::max)(newInnerAngle, newOuterAngle);
    }
};