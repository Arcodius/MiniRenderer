#include "Renderer.h"

#include <array>   // Include <array> for std::array usage
#include "Color.h"
#include "Intersection.h"
#include "Material.h"
#include "Ray.h"
#include "ResourceManager.h"
#include "Scene.h"


//#include "Debug.h"
//#define DEBUG_MODE

void Renderer::clearBuffers() {
	framebuffer.clear(0xFF000000);
	zbuffer.clear(std::numeric_limits<float>::max());  // Clear depth buffer to max depth
}
// w is world position
bool Renderer::_isBackFacingViewSpace(
    const glm::vec3& w0, const glm::vec3& w1, const glm::vec3& w2,
    const glm::vec3& cameraPosition)
{
    glm::vec3 faceNormal = glm::normalize(glm::cross(w1 - w0, w2 - w0));
    glm::vec3 toCamera = glm::normalize(cameraPosition - w0);
    return glm::dot(faceNormal, toCamera) < 0.0f; // 背向摄像机则为 true
}


bool Renderer::_insideTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    return (glm::cross(b - a, p - a).z >= 0.0f &&
            glm::cross(c - b, p - b).z >= 0.0f &&
            glm::cross(a - c, p - c).z >= 0.0f);
}

std::vector<glm::vec3> Renderer::clipToScreen(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, int screenWidth, int screenHeight) {
    std::vector<glm::vec3> vertices = {v0, v1, v2};
    std::vector<glm::vec3> clippedVertices;

    // 裁剪到屏幕范围 [0, screenWidth] 和 [0, screenHeight]
    for (size_t i = 0; i < vertices.size(); ++i) {
        const glm::vec3& v = vertices[i];
        if (v.x >= 0 && v.x < screenWidth && v.y >= 0 && v.y < screenHeight) {
            clippedVertices.push_back(v);
        }
    }

    return clippedVertices;
}

VertexShaderOutput vertexShader(const Vertex& in, const glm::mat4& model, const glm::mat3& normalMat, const glm::mat4& mvp) {
    VertexShaderOutput out;
    glm::vec4 worldPos4 = model * glm::vec4(in.localPos, 1.0f);
    out.clipPos = mvp * glm::vec4(in.localPos, 1.0f);
    out.worldPos = glm::vec3(worldPos4);
    out.normal = glm::normalize(normalMat * in.normal);
    out.uv = in.uv;
    out.w = out.clipPos.w;
    return out;
}

VertexShaderOutput interpolate(const VertexShaderOutput& a, const VertexShaderOutput& b, float alpha) {
    VertexShaderOutput out;
    out.clipPos = glm::mix(a.clipPos, b.clipPos, alpha);
    out.worldPos = glm::mix(a.worldPos, b.worldPos, alpha);
    out.normal = glm::normalize(glm::mix(a.normal, b.normal, alpha));
    out.uv = glm::mix(a.uv, b.uv, alpha);
    out.w = glm::mix(a.w, b.w, alpha);
    return out;
}

// v: vertex
// s: point in screen space
// w: world space position
void Renderer::_drawTrianglePhong(
    const VertexShaderOutput& v0, const VertexShaderOutput& v1, const VertexShaderOutput& v2,
    const glm::vec3& s0, const glm::vec3& s1, const glm::vec3& s2,
    const std::vector<std::shared_ptr< Light >>& lights, const Camera& camera, std::shared_ptr<Material> material) {
    // if (w0 < EPSILON || w1 < EPSILON || w2 < EPSILON) {
    //     // printf("skipped point: w0=%.2f, w1=%.2f, w2=%.2f\n", w0, w1, w2);
    //     return; // 避免除以零
    // }

    // 保证顺时针方向，防止 area 负值带来插值错误
    float area = glm::cross(s1 - s0, s2 - s0).z;
    if (fabs(area) < EPSILON) {
        // printf("Degenerate triangle: area=%.2f\n", area);
        return; // 退化三角形
    } // 退化三角形
    if (area < 0.0f) {
        _drawTrianglePhong(v0, v2, v1, s0, s2, s1, lights, camera, material);
        return;
    }
    float invW0 = 1.0f / v0.w;
    float invW1 = 1.0f / v1.w;
    float invW2 = 1.0f / v2.w;
    glm::vec2 uv0_w = v0.uv * invW0;
    glm::vec2 uv1_w = v1.uv * invW1;
    glm::vec2 uv2_w = v2.uv * invW2;
    glm::vec3 n0_w = v0.normal * invW0;
    glm::vec3 n1_w = v1.normal * invW1;
    glm::vec3 n2_w = v2.normal * invW2;

    int minX = std::max(0, (int)std::floor(std::min({ s0.x, s1.x, s2.x })));
    int maxX = std::min(screenWidth - 1, (int)std::ceil(std::max({ s0.x, s1.x, s2.x })));
    int minY = std::max(0, (int)std::floor(std::min({ s0.y, s1.y, s2.y })));
    int maxY = std::min(screenHeight - 1, (int)std::ceil(std::max({ s0.y, s1.y, s2.y })));

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            glm::vec3 p(x + 0.5f, y + 0.5f, 0.0f);
            if (_insideTriangle(p, s0, s1, s2)) {
                float a = (glm::cross(s1 - p, s2 - p)).z / area;
                float b = (glm::cross(s2 - p, s0 - p)).z / area;
                float c = (glm::cross(s0 - p, s1 - p)).z / area;

                float z = a * s0.z + b * s1.z + c * s2.z;
                int idx = y * screenWidth + x;
                
                if (z < zbuffer[idx]) {
                    zbuffer[idx] = z;
                    glm::vec3 pos = v0.worldPos * a + v1.worldPos * b + v2.worldPos * c;
                    glm::vec3 normal = glm::normalize(n0_w * a + n1_w * b + n2_w * c);

                    // 透视修正插值 uv
                    float invW = a * invW0 + b * invW1 + c * invW2;
                    glm::vec2 uv = (a * uv0_w + b * uv1_w + c * uv2_w) / invW;
                    // glm::vec3 baseColor = sampleTexture(textureData, uv, textureWidth, textureHeight);
                    glm::vec3 baseColor = material->sampleBaseColor(uv);
                    glm::vec3 color = glm::vec3(0.0f);
                    for (const auto& light : lights) {
                        if (light->getDistance(pos) < EPSILON) continue; // 避免光源距离过近
                        color += _computePhongColor(pos, normal, light, camera.getPosition(), baseColor);
                    }
                    color = glm::clamp(color, 0.0f, 1.0f); // 确保颜色在 [0, 1] 范围内
                    framebuffer[idx] = Color::VecToUint32(color); // 映射 [-1,1] → [0,1]
                }
            }
        }
    }
}

glm::vec3 Renderer::_computePhongColor(const glm::vec3& pos, const glm::vec3& normal, const std::shared_ptr<Light>& light, const glm::vec3& cameraPos, const glm::vec3& baseColor)
{
	glm::vec3 L = light->getDirection(pos);
	glm::vec3 N = glm::normalize(normal);
	glm::vec3 V = glm::normalize(cameraPos - pos);
	glm::vec3 R = glm::normalize(2.0f * glm::dot(N, L) * N - L);

	float diff = glm::max(0.0f, glm::dot(N, L)); // Lambertian reflectance
	float spec = std::pow(glm::max(0.0f, glm::dot(R, V)), 16.0f); // shininess

	glm::vec3 ambient = 0.1f * baseColor;
	glm::vec3 diffuse = diff * baseColor;
	glm::vec3 specular = spec * glm::vec3{ 1,1,1 }; // white specular

	glm::vec3 color = (ambient + diffuse + specular)*light->getIntensity(pos);
	return glm::clamp(color, 0.0f, 1.0f);
}

glm::vec3 Renderer::sampleTexture(const std::vector<uint32_t>& textureData, glm::vec2 uv, int texWidth, int texHeight) {
    int texX = CLAMP(int(uv.x * texWidth), 0, texWidth - 1);
    int texY = CLAMP(int(uv.y * texHeight), 0, texHeight - 1);
    return Color::Uint32ToVec(textureData[texY * texWidth + texX]); // 纹理采样
}

glm::vec3 Renderer::ndcToScreen(const glm::vec3& ndc) const {
    float x = (ndc.x + 1.0f) * 0.5f * screenWidth;
    float y = (1.0f - ndc.y) * 0.5f * screenHeight; // 注意y翻转
    float z = (ndc.z + 1.0f) * 0.5f; // depth: [0, 1] for z-buffer
    return glm::vec3(x, y, z);
}


void Renderer::clip_triangle_against_near_plane(
    const VertexShaderOutput& v0, const VertexShaderOutput& v1, const VertexShaderOutput& v2,
    std::vector<std::array<VertexShaderOutput, 3>>& clipped_tris)
{
    const VertexShaderOutput vertices[] = { v0, v1, v2 };
    std::vector<VertexShaderOutput> inside_points;
    std::vector<VertexShaderOutput> outside_points;

    for (int i = 0; i < 3; ++i) {
        if (vertices[i].clipPos.w >= EPSILON) {
            inside_points.push_back(vertices[i]);
        } else {
            outside_points.push_back(vertices[i]);
        }
    }

    if (inside_points.size() == 3) {
        clipped_tris.push_back({ v0, v1, v2 });
    }
    else if (inside_points.size() == 1 && outside_points.size() == 2) {
        const VertexShaderOutput& in_v = inside_points[0];
        const VertexShaderOutput& out_v0 = outside_points[0];
        const VertexShaderOutput& out_v1 = outside_points[1];

        float t0 = (EPSILON - in_v.clipPos.w) / (out_v0.clipPos.w - in_v.clipPos.w);
        float t1 = (EPSILON - in_v.clipPos.w) / (out_v1.clipPos.w - in_v.clipPos.w);

        VertexShaderOutput new_v0 = interpolate(in_v, out_v0, t0);
        VertexShaderOutput new_v1 = interpolate(in_v, out_v1, t1);

        clipped_tris.push_back({ in_v, new_v0, new_v1 });
    }
    else if (inside_points.size() == 2 && outside_points.size() == 1) {
        const VertexShaderOutput& in_v0 = inside_points[0];
        const VertexShaderOutput& in_v1 = inside_points[1];
        const VertexShaderOutput& out_v = outside_points[0];

        float t0 = (EPSILON - in_v0.clipPos.w) / (out_v.clipPos.w - in_v0.clipPos.w);
        float t1 = (EPSILON - in_v1.clipPos.w) / (out_v.clipPos.w - in_v1.clipPos.w);

        VertexShaderOutput new_v0 = interpolate(in_v0, out_v, t0);
        VertexShaderOutput new_v1 = interpolate(in_v1, out_v, t1);

        clipped_tris.push_back({ in_v0, in_v1, new_v0 });
        clipped_tris.push_back({ in_v1, new_v1, new_v0 });
    }
}

void Renderer::render(Scene scene) {
    clearBuffers();
    scene.camera.setAspect(static_cast<float>(screenWidth) / screenHeight);
    glm::mat4 projectionMatrix = scene.camera.getProjectionMatrix();
    glm::mat4 viewMatrix = scene.camera.getViewMatrix();
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
    for (const auto& objectPtr : scene.objects) {
        Object& object = *objectPtr;
        glm::mat4 modelMatrix = object.getMatrix();
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
        glm::mat4 mvp = viewProjectionMatrix * modelMatrix;

        const Mesh& mesh = object.getMesh();
        const std::vector<Vertex>& vertices = mesh.vertices;
        const std::vector<unsigned int>& indices = mesh.indices;

        #pragma omp parallel for
        for (size_t i = 0; i < indices.size(); i += 3) {
            VertexShaderOutput v0 = vertexShader(vertices[indices[i]], modelMatrix, normalMatrix, mvp);
            VertexShaderOutput v1 = vertexShader(vertices[indices[i+1]], modelMatrix, normalMatrix, mvp);
            VertexShaderOutput v2 = vertexShader(vertices[indices[i+2]], modelMatrix, normalMatrix, mvp);

            // 剪裁（保留完整结构体）
            std::vector<std::array<VertexShaderOutput, 3>> clippedTriangles;
            clip_triangle_against_near_plane(v0, v1, v2, clippedTriangles);

            for (const auto& tri : clippedTriangles) {
                // Perspective divide and viewport transform
                glm::vec3 s0 = ndcToScreen(tri[0].clipPos / tri[0].clipPos.w);
                glm::vec3 s1 = ndcToScreen(tri[1].clipPos / tri[1].clipPos.w);
                glm::vec3 s2 = ndcToScreen(tri[2].clipPos / tri[2].clipPos.w);

                _drawTrianglePhong(tri[0], tri[1], tri[2], s0, s1, s2, scene.lights, scene.camera, object.getMaterial());
            }
        }
    }
}

void Renderer::renderRayTracing(Scene scene) {
    clearBuffers();

    // Iterate over each pixel in the framebuffer
    for (int y = 0; y < screenHeight; ++y) {
        for (int x = 0; x < screenWidth; ++x) {
            // Generate ray for the current pixel
            Ray ray = scene.camera.generateRay(x, y, screenWidth, screenHeight);
            // Trace the ray through the scene
            glm::vec3 color = traceRay(ray, scene, 0);
            // Write the color to the framebuffer
            framebuffer.setPixel(x, y, Color::VecToUint32(color));
        }
    }
}

glm::vec3 Renderer::traceRay(const Ray& ray, const Scene& scene, int depth) {
    if (depth > MAX_DEPTH) {
        return glm::vec3(0.0f);
    }

    Intersection intersection;
    if (!scene.intersect(ray, intersection)) {
        return scene.getBackgroundColor();
    }
    const Material& mat = *intersection.material;

    glm::vec3 color(0.0f);

    // === 1. compute BRDF-based local shading ===
    glm::vec3 viewDir = glm::normalize(scene.camera.getPosition() - intersection.position);
    for (const auto& light : scene.lights) {
        glm::vec3 lightDir = light->getDirection(intersection.position);
        glm::vec3 lightColor = light->getColor() * light->getIntensity(intersection.position);
        if (isInShadow(intersection.position, scene, light->getPosition())) {
            continue; // Skip if in shadow
        }
        color += mat.computeBRDF(intersection.normal, viewDir, lightDir, lightColor);
    }

    // === 2. optionally compute reflection for low roughness metals ===
    glm::vec3 reflectionColor(0.0f);
    if (mat.metallic > 0.0f && mat.roughness < 0.2f) {
        glm::vec3 reflectDir = glm::reflect(ray.direction, intersection.normal);
        Ray reflectedRay(intersection.position + reflectDir * 1e-4f, reflectDir);
        reflectionColor = traceRay(reflectedRay, scene, depth + 1);
        
        // Fresnel factor from Schlick (reuse logic inside computeBRDF if needed)
        float NdotV = glm::dot(intersection.normal, -ray.direction);
        glm::vec3 F0 = glm::mix(glm::vec3(0.04f), mat.baseColor, mat.metallic);
        glm::vec3 fresnel = F0 + (1.0f - F0) * glm::pow(1.0f - NdotV, 5.0f);

        color += fresnel * reflectionColor; // Add specular reflection
    }

    // === 3. optionally compute refraction for transparent dielectrics ===
    if (mat.transparency > 0.0f) {
        Ray refractedRay = computeRefractedRay(ray, intersection);
        glm::vec3 refractedColor = traceRay(refractedRay, scene, depth + 1);
        color = glm::mix(color, refractedColor, mat.transparency);
    }

    return glm::clamp(color, 0.0f, 1.0f);
}


Ray Renderer::computeReflectedRay(const Ray& ray, const Intersection& isect) {
    glm::vec3 normal = isect.normal;
    glm::vec3 incident = glm::normalize(ray.direction);
    glm::vec3 reflectedDir = glm::reflect(incident, normal);
    // 防止浮点精度问题导致自交（"acne"），原点稍作偏移
    glm::vec3 origin = isect.position + reflectedDir * 1e-4f;
    return Ray(origin, reflectedDir);
}

Ray Renderer::computeRefractedRay(const Ray& ray, const Intersection& isect) {
    glm::vec3 incident = glm::normalize(ray.direction);
    glm::vec3 normal = isect.normal;
    float eta = isect.material->ior;

    // 判断是否从内部射出
    float cosi = glm::dot(incident, normal);
    float etai = 1.0f, etat = eta;
    if (cosi > 0.0f) {
        std::swap(etai, etat);
        normal = -normal;
    } else {
        cosi = -cosi;
    }

    float etaRatio = etai / etat;
    float k = 1.0f - etaRatio * etaRatio * (1.0f - cosi * cosi);

    glm::vec3 refractedDir;
    if (k < 0.0f) {
        // 全反射，全反射时不发射折射光线
        refractedDir = glm::reflect(incident, normal);
    } else {
        refractedDir = etaRatio * incident + (etaRatio * cosi - sqrtf(k)) * normal;
    }

    glm::vec3 origin = isect.position + refractedDir * 1e-4f;
    return Ray(origin, refractedDir);
}

float Renderer::fresnelSchlick(float cosTheta, float ior) {
    float r0 = (1.0f - ior) / (1.0f + ior);
    r0 = r0 * r0;
    return r0 + (1.0f - r0) * pow(1.0f - cosTheta, 5.0f);
}


// bool Renderer::isInShadow(const glm::vec3& point, const Scene& scene, const glm::vec3& lightPos) {
//     glm::vec3 shadowRayDir = glm::normalize(lightPos - point);
//     Ray shadowRay(point + shadowRayDir * 0.001f, shadowRayDir); // 偏移避免自遮挡
//     Intersection shadowIsect;
//     shadowIsect.t = glm::length(lightPos - point);
//     return scene.intersect(shadowRay, shadowIsect); // 如果有遮挡，返回 true
// }

bool Renderer::isInShadow(const glm::vec3& point, const Scene& scene, const glm::vec3& lightPos) {
    // 计算从着色点到光源的向量和距离
    glm::vec3 toLightDir = lightPos - point;
    float distanceToLight = glm::length(toLightDir);
    toLightDir = glm::normalize(toLightDir);

    // 创建一条阴影光线
    // 关键：将光线的最远行程设置为到光源的距离
    // 任何超过这个距离的交点都无关紧要（因为它们在光源后面
    Ray shadowRay(point, toLightDir);
    shadowRay.t_max = distanceToLight;

    // 调用新的、高效的遮挡函数
    return scene.hasIntersection(shadowRay);
}

float Renderer::computeSoftShadow(const glm::vec3& point, const Scene& scene, const glm::vec3& lightPos, int numSamples) {
    int occludedSamples = 0;
    for (int i = 0; i < numSamples; ++i) {
        glm::vec3 jitteredLightPos = lightPos + glm::vec3(
            (rand() % 100 / 100.0f - 0.5f) * 0.1f, // 随机偏移
            (rand() % 100 / 100.0f - 0.5f) * 0.1f,
            (rand() % 100 / 100.0f - 0.5f) * 0.1f
        );
        if (isInShadow(point, scene, jitteredLightPos)) {
            occludedSamples++;
        }
    }
    return 1.0f - float(occludedSamples) / numSamples; // 返回未遮挡比例
}

Renderer::Renderer(int width, int height){
    screenWidth = width;
    screenHeight = height;
    framebuffer = Buffer<uint32_t>(width, height);
    zbuffer = Buffer<float>(width, height);
    clearBuffers();
}
