#include "Material.h"

#include "ResourceLoader.h"

void Material::loadDiffuseTexture(const std::string& path) {
    diffuseTexture = ResourceLoader::loadTextureFromFile(path, textureWidth, textureHeight);
    if (diffuseTexture.empty()) {
        throw std::runtime_error("Failed to load diffuse texture: " + path);
    }
}

void Material::loadSpecularTexture(const std::string& path) {
    specularTexture = ResourceLoader::loadTextureFromFile(path, textureWidth, textureHeight);
    if (specularTexture.empty()) {
        throw std::runtime_error("Failed to load specular texture: " + path);
    }
}

glm::vec3 Material::sampleDiffuseTexture(const glm::vec2& uv) const {
    if (diffuseTexture.empty()) {
        return diffuseColor; // Return base color if no texture is loaded
    }

    int texX = std::clamp(int(uv.x * textureWidth), 0, textureWidth - 1);
    int texY = std::clamp(int(uv.y * textureHeight), 0, textureHeight - 1);
    uint32_t pixel = diffuseTexture[texY * textureWidth + texX];

    return glm::vec3(
        (pixel & 0xFF) / 255.0f,         // Red
        ((pixel >> 8) & 0xFF) / 255.0f, // Green
        ((pixel >> 16) & 0xFF) / 255.0f // Blue
    );
}

glm::vec3 Material::sampleSpecularTexture(const glm::vec2& uv) const {
    if (specularTexture.empty()) {
        return specularColor; // Return base specular color if no texture is loaded
    }

    int texX = std::clamp(int(uv.x * textureWidth), 0, textureWidth - 1);
    int texY = std::clamp(int(uv.y * textureHeight), 0, textureHeight - 1);
    uint32_t pixel = specularTexture[texY * textureWidth + texX];

    return glm::vec3(
        (pixel & 0xFF) / 255.0f,         // Red
        ((pixel >> 8) & 0xFF) / 255.0f, // Green
        ((pixel >> 16) & 0xFF) / 255.0f // Blue
    );
}

glm::vec3 Material::computeBRDF(
    const glm::vec3& normal,
    const glm::vec3& viewDir,
    const glm::vec3& lightDir,
    const glm::vec3& lightColor) const {

    // Fresnel term (Schlick approximation)
    glm::vec3 halfDir = glm::normalize(viewDir + lightDir);
    float NdotH = std::max(glm::dot(normal, halfDir), 0.0f);
    glm::vec3 F = specularColor + (glm::vec3(1.0f) - specularColor) * glm::pow(1.0f - NdotH, 5.0f);

    // Geometry term
    float NdotV = std::max(glm::dot(normal, viewDir), 0.0f);
    float NdotL = std::max(glm::dot(normal, lightDir), 0.0f);
    float k = roughness * roughness / 2.0f;
    float G = (NdotV * NdotL) / (NdotV * (1.0f - k) + k) * (NdotL * (1.0f - k) + k);

    // Distribution term (GGX)
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = NdotH * NdotH * (alpha2 - 1.0f) + 1.0f;
    float D = alpha2 / (glm::pi<float>() * denom * denom);

    // BRDF
    glm::vec3 specular = (F * G * D) / (4.0f * NdotV * NdotL + 1e-5f);
    glm::vec3 diffuse = (1.0f - metallic) * diffuseColor / glm::pi<float>();

    return lightColor * (diffuse + specular) * NdotL;
}