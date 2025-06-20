#pragma once

#include <random>
static std::mt19937 generator(std::random_device{}());
static std::uniform_real_distribution<float> distribution(-0.5f, 0.5f);

#include "MyMath.h"

class Light {
public:
	glm::vec3 color;
	float intensity;

	Light() : color(glm::vec3(1.0f)), intensity(1.0f) {}
	Light(const glm::vec3& color_, float intensity_) : color(color_), intensity(intensity_) {}
    virtual ~Light() = default;

	glm::vec3 getColor() const { return color; }
    virtual glm::vec3 getPosition() const = 0;
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

    glm::vec3 getPosition() const override {
        return glm::vec3(0.0f); // Directional light does not have a position
    }

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

    // Point light emits light in all directions, so direction is from point to light source
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



class AreaLight : public Light {
public:
    glm::vec3 position; // 矩形的中心点
    glm::vec3 u_dir;    // 矩形的一条边方向（单位向量）
    glm::vec3 v_dir;    // 矩形的另一条边方向（单位向量）
    float width;      // u_dir 方向的长度
    float height;     // v_dir 方向的长度
    glm::vec3 normal;   // 区域光的法线方向

    AreaLight(const glm::vec3& color_, float intensity_,
              const glm::vec3& position_,
              const glm::vec3& u_dir_, const glm::vec3& v_dir_,
              float width_, float height_)
        : Light(color_, intensity_), position(position_),
          u_dir(glm::normalize(u_dir_)), v_dir(glm::normalize(v_dir_)),
          width(width_), height(height_) {
        // 计算法线，假设 u_dir 和 v_dir 是垂直的
        normal = glm::normalize(glm::cross(u_dir, v_dir));
    }

    // --- 核心变化在这里 ---
    // 我们需要一个函数来在光照表面上采样一个随机点
    glm::vec3 samplePointOnLight() const {
        float rand_u = distribution(generator); // 在 [-0.5, 0.5] 范围内生成随机数
        float rand_v = distribution(generator);
        return position + (rand_u * width * u_dir) + (rand_v * height * v_dir);
    }

    // getPosition() 可以返回区域光的中心
    glm::vec3 getPosition() const override {
        return position;
    }

    // 注意：这些函数现在依赖于一个从外部传入的“采样点”
    // 为了保持接口一致，我们在这里进行采样。
    // 一个更好的设计可能是修改渲染循环，但我们先适配现有接口。

    // 方向是从着色点指向光源上的一个*随机采样点*
    glm::vec3 getDirection(const glm::vec3& point) const override {
        glm::vec3 lightSamplePos = samplePointOnLight();
        return glm::normalize(lightSamplePos - point);
    }
    
    // 强度计算也基于采样点，并且要考虑光照法线
    float getIntensity(const glm::vec3& point) const override {
        // 1. 在区域光上采样一个点
        glm::vec3 lightSamplePos = samplePointOnLight();
        
        // 2. 计算距离和方向
        glm::vec3 lightDir = lightSamplePos - point;
        float distance2 = glm::dot(lightDir, lightDir);
        lightDir = glm::normalize(lightDir);

        // 3. 计算衰减 (与 PointLight 类似)
        float attenuation = 1.0f / (distance2 + 1.0f); // 也可以用你的公式

        // 4. 计算余弦项
        // 光线只会从法线方向射出，所以我们需要检查夹角
        // dot(lightNormal, -lightDir) 表示光线出射方向和法线的夹角余弦
        float cos_theta = glm::dot(normal, -lightDir);
        
        // // 如果点在区域光的背面，则光照强度为0
        if (cos_theta <= 0.0f) {
            return 0.0f;
        }

        // 5. 最终强度
        // 乘以 cos_theta 是物理正确的（朗伯余弦定律）
        return intensity * attenuation * cos_theta;
    }

    // 距离也是到着色点的距离
    float getDistance(const glm::vec3& point) const override {
        // 返回到中心的距离，主要用于粗略判断
        return glm::distance(point, position);
    }
};