#include "Material.h"

#include "Color.h"
#include "ResourceManager.h"

void Material::loadBaseColorMap(const std::string& path) {
    colorMap = ResourceManager::loadTextureFromFile(path, textureWidth, textureHeight);
    if (colorMap.empty()) {
        throw std::runtime_error("Failed to load diffuse texture: " + path);
    }
}

void Material::loadRoughnessMap(const std::string& path) {
    roughnessMap = ResourceManager::loadTextureFromFile(path, textureWidth, textureHeight);
    if (roughnessMap.empty()) {
        throw std::runtime_error("Failed to load specular texture: " + path);
    }
}

void Material::loadNormalMap(const std::string& path) {
    normalMap = ResourceManager::loadTextureFromFile(path, textureWidth, textureHeight);
    if (normalMap.empty()) {
        throw std::runtime_error("Failed to load normal map texture: " + path);
    }
}

glm::vec3 Material::sampleBaseColor(const glm::vec2& uv) const {
    if (colorMap.empty()) {
        return baseColor; // Return base color if no texture is loaded
    }

    int texX = CLAMP(int(uv.x * textureWidth), 0, textureWidth - 1);
    int texY = CLAMP(int(uv.y * textureHeight), 0, textureHeight - 1);
    uint32_t pixel = colorMap[texY * textureWidth + texX];

    return Color::Uint32ToVec(pixel); // Convert pixel to vec3
}

float Material::sampleRoughness(const glm::vec2& uv) const {
    if (roughnessMap.empty()) {
        return roughness; // Return base roughness if no texture is loaded
    }

    int texX = CLAMP(int(uv.x * textureWidth), 0, textureWidth - 1);
    int texY = CLAMP(int(uv.y * textureHeight), 0, textureHeight - 1);
    uint32_t pixel = roughnessMap[texY * textureWidth + texX];

    // Extract the red channel for grayscale roughness
    float redChannel = (pixel & 0xFF) / 255.0f; // Normalize to [0, 1]
    return redChannel;
}

glm::vec3 Material::computePhong(
    const glm::vec3& normal,
    const glm::vec2& uv,
    const glm::vec3& viewDir,
    const glm::vec3& lightDir,
    const glm::vec3& lightColor) const {

    // 计算半程向量
    glm::vec3 halfVector = glm::normalize(viewDir + lightDir);

    // 计算光照相关的点积
    float NdotL = glm::max(glm::dot(normal, lightDir), 0.0f); // 法线与光线方向的点积
    float NdotV = glm::max(glm::dot(normal, viewDir), 0.0f); // 法线与视线方向的点积
    float NdotH = glm::max(glm::dot(normal, halfVector), 0.0f); // 法线与半程向量的点积

    // 漫反射项 (Lambertian)
    glm::vec3 diffuse = sampleBaseColor(uv) * NdotL;

    // 高光项 (Phong)
    float shininess = glm::max(roughness * 128.0f, 1.0f); // 将粗糙度映射到 Phong 模型的 shininess
    float specularStrength = metallic; // 使用 metallic 作为高光强度
    float spec = glm::pow(NdotH, shininess); // Phong 高光公式
    glm::vec3 specular = specularStrength * spec * lightColor;

    // 组合漫反射和高光
    glm::vec3 result = diffuse + specular;

    // 乘以光源颜色和强度
    result *= lightColor;

    // 确保结果在 [0, 1] 范围内
    return glm::clamp(result, 0.0f, 1.0f);
}

glm::vec3 Material::computeBRDF(
    const glm::vec3& normal,
    const glm::vec3& viewDir,
    const glm::vec3& lightDir,
    const glm::vec3& lightColor) const {

    glm::vec3 halfVector = glm::normalize(viewDir + lightDir);

    float NdotL = glm::max(glm::dot(normal, lightDir), 0.0f);
    float NdotV = glm::max(glm::dot(normal, viewDir), 0.0f);
    float NdotH = glm::max(glm::dot(normal, halfVector), 0.0f);
    float VdotH = glm::max(glm::dot(viewDir, halfVector), 0.0f);

    // Fresnel (Schlick)
    glm::vec3 F0 = glm::mix(glm::vec3(0.04f), baseColor, metallic);
    glm::vec3 F = F0 + (1.0f - F0) * glm::pow(1.0f - VdotH, 5.0f);

    // GGX Normal Distribution
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0f) + 1.0f;
    float D = a2 / (glm::pi<float>() * denom * denom);

    // Geometry Smith
    auto G1 = [](float NdotX, float k) {
        return NdotX / (NdotX * (1.0f - k) + k);
    };
    float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
    float G = G1(NdotL, k) * G1(NdotV, k);

    // Specular term
    glm::vec3 specular = (D * G * F) / (4.0f * NdotV * NdotL + 1e-4f);

    // Diffuse term (Lambert, only for non-metals)
    glm::vec3 diffuse = (1.0f - F) * baseColor / glm::pi<float>();
    diffuse *= (1.0f - metallic);

    glm::vec3 result = (diffuse + specular) * lightColor * NdotL;

    return glm::clamp(result, 0.0f, 1.0f);
}

glm::vec3 Material::computeBRDF(
    const glm::vec3& normal,
    const glm::vec2& uv,
    const glm::vec3& viewDir,
    const glm::vec3& lightDir,
    const glm::vec3& lightColor) const {

    glm::vec3 halfVector = glm::normalize(viewDir + lightDir);
    
    float NdotL = glm::max(glm::dot(normal, lightDir), 0.0f);
    float NdotV = glm::max(glm::dot(normal, viewDir), 0.0f);
    float NdotH = glm::max(glm::dot(normal, halfVector), 0.0f);
    float VdotH = glm::max(glm::dot(viewDir, halfVector), 0.0f);

    glm::vec3 texColor = sampleBaseColor(uv);
    float texRoughness = sampleRoughness(uv);

    // Fresnel (Schlick)
    glm::vec3 F0 = glm::mix(glm::vec3(0.04f), texColor, metallic);
    glm::vec3 F = F0 + (1.0f - F0) * glm::pow(1.0f - VdotH, 5.0f);

    // GGX Normal Distribution
    float a = texRoughness * texRoughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0f) + 1.0f;
    float D = a2 / (glm::pi<float>() * denom * denom);

    // Geometry Smith
    auto G1 = [](float NdotX, float k) {
        return NdotX / (NdotX * (1.0f - k) + k);
    };
    float k = (texRoughness + 1.0f) * (texRoughness + 1.0f) / 8.0f;
    float G = G1(NdotL, k) * G1(NdotV, k);

    // Specular term
    glm::vec3 specular = (D * G * F) / (4.0f * NdotV * NdotL + 1e-4f);

    // Diffuse term (Lambert, only for non-metals)
    glm::vec3 diffuse = (1.0f - F) * texColor / glm::pi<float>();
    diffuse *= (1.0f - metallic);

    glm::vec3 result = (diffuse + specular) * lightColor * NdotL;

    return glm::clamp(result, 0.0f, 1.0f);
}
