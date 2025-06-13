#pragma once

#include <memory>
#include <string>
#include <vector>

#include "MyMath.h"

class Material {
    private:
    std::string name;
public:
    // Basic properties
    glm::vec3 diffuseColor;    // Diffuse color
    float reflectivity;        // 0 to 1
    float transparency;        // 0 to 1
    float refractiveIndex;     // >1 for real materials

    // BRDF-related properties
    glm::vec3 specularColor;   // Specular color
    float roughness;           // Roughness (0 = perfect mirror, 1 = fully diffuse)
    float metallic;            // Metallic factor (0 = non-metal, 1 = pure metal)

    // Texture maps
    std::vector<uint32_t> diffuseTexture;  // Diffuse texture
    std::vector<uint32_t> specularTexture; // Specular texture
    int textureWidth = 0, textureHeight = 0; // Texture dimensions

    Material(std::string name = "default")
        : name(name), diffuseColor(1.0f), reflectivity(0.0f), transparency(0.0f), refractiveIndex(1.0f),
          specularColor(1.0f), roughness(0.5f), metallic(0.0f) {}
    
    std::string get_name() const { return name; }

    // Load texture data
    void loadDiffuseTexture(const std::string& path);
    void loadSpecularTexture(const std::string& path);

    // Sample texture at UV coordinates
    glm::vec3 sampleDiffuseTexture(const glm::vec2& uv) const;
    glm::vec3 sampleSpecularTexture(const glm::vec2& uv) const;

    // BRDF computation (Cook-Torrance model)
    glm::vec3 computeBRDF(
        const glm::vec3& normal,
        const glm::vec3& viewDir,
        const glm::vec3& lightDir,
        const glm::vec3& lightColor) const;
};
