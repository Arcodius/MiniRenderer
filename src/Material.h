#pragma once

#include <memory>
#include <string>
#include <vector>

#include "MyMath.h"

class Material {
private:
    std::string name;

public:
    // PBR Core
    glm::vec3 baseColor;   // aka albedo
    float metallic;        // 0 = dielectric, 1 = metal
    float roughness;       // 0 = smooth, 1 = rough
    float ior;             // Index of Refraction (dielectrics only)
    float transparency;    // 0 = opaque, 1 = fully transparent

    // Optional maps
    std::vector<uint32_t> colorMap;
    std::vector<uint32_t> roughnessMap;
    std::vector<uint32_t> normalMap;
    int textureWidth = 0, textureHeight = 0;

    Material(const std::string& name = "default")
        : name(name),
          baseColor(0.8f), metallic(0.0f), roughness(0.5f),
          ior(1.5f), transparency(0.0f) {}

    std::string get_name() const { return name; }

    // Load texture data
    void loadBaseColorMap(const std::string& path);
    void loadRoughnessMap(const std::string& path);
    void loadNormalMap(const std::string& path);

    // Texture sampling
    glm::vec3 sampleBaseColor(const glm::vec2& uv) const;
    float sampleRoughness(const glm::vec2& uv) const;

    // Phong shading
    glm::vec3 computePhong(
        const glm::vec3& normal,
        const glm::vec2& uv,
        const glm::vec3& viewDir,
        const glm::vec3& lightDir,
        const glm::vec3& lightColor) const;

    // Core BRDF (Cook-Torrance)
    glm::vec3 computeBRDF(
        const glm::vec3& normal,
        const glm::vec3& viewDir,
        const glm::vec3& lightDir,
        const glm::vec3& lightColor) const;
    glm::vec3 computeBRDF(
        const glm::vec3& normal,
        const glm::vec2& uv,
        const glm::vec3& viewDir,
        const glm::vec3& lightDir,
        const glm::vec3& lightColor) const;
    
    static std::shared_ptr<Material> defualtMat() { return std::make_shared<Material>("default"); }
};
