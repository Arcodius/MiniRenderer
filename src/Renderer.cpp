#include "Renderer.h"

#include <array>   // Include <array> for std::array usage
#include "Color.h"
#include "Intersection.h"
#include "Ray.h"
#include "ResourceLoader.h"
#include "Scene.h"
#include "Line.h" // Ensure the definition of Line is included


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
// Helper to interpolate vertex attributes for new vertices created during clipping
Vertex interpolate_vertex(const Vertex& v0, const Vertex& v1, float t) {
    Vertex result;
    result.worldPos = lerp(v0.worldPos, v1.worldPos, t);
    result.normal = glm::normalize(lerp(v0.normal, v1.normal, t));
    result.uv = lerp(v0.uv, v1.uv, t);
    // Note: Other vertex attributes like color would be interpolated here too.
    return result;
}

// Clips a triangle against the w=EPSILON plane in clip space.
// Outputs 0, 1, or 2 triangles into the `clipped_triangles` vector.
void clip_triangle_against_near_plane(
    const Vertex& v0, const Vertex& v1, const Vertex& v2,
    const glm::vec4& clip0, const glm::vec4& clip1, const glm::vec4& clip2,
    std::vector<std::array<Vertex, 3>>& clipped_triangles)
{
    // Store vertices and their clip-space counterparts
    const Vertex vertices[] = { v0, v1, v2 };
    const glm::vec4 clip_coords[] = { clip0, clip1, clip2 };

    std::vector<Vertex> inside_points;
    std::vector<Vertex> outside_points;
    
    // Classify vertices
    for (int i = 0; i < 3; ++i) {
        if (clip_coords[i].w >= EPSILON) {
            inside_points.push_back(vertices[i]);
        } else {
            outside_points.push_back(vertices[i]);
        }
    }

    // Based on the number of inside points, generate new triangles
    if (inside_points.size() == 3) {
        // Case 1: All vertices are inside, keep original triangle
        clipped_triangles.push_back({ v0, v1, v2 });
    }
    else if (inside_points.size() == 1 && outside_points.size() == 2) {
        // Case 2: One vertex inside, form one new triangle
        const Vertex& in_v = inside_points[0];
        const Vertex& out_v0 = outside_points[0];
        const Vertex& out_v1 = outside_points[1];
        
        // Find corresponding clip coordinates to calculate interpolation factor 't'
        auto find_clip = [&](const Vertex& v) { 
            if (v.worldPos == v0.worldPos) return clip0;
            if (v.worldPos == v1.worldPos) return clip1;
            return clip2;
        };

        glm::vec4 in_c = find_clip(in_v);
        glm::vec4 out_c0 = find_clip(out_v0);
        glm::vec4 out_c1 = find_clip(out_v1);

        // Calculate intersection points
        float t0 = (EPSILON - in_c.w) / (out_c0.w - in_c.w);
        Vertex new_v0 = interpolate_vertex(in_v, out_v0, t0);

        float t1 = (EPSILON - in_c.w) / (out_c1.w - in_c.w);
        Vertex new_v1 = interpolate_vertex(in_v, out_v1, t1);

        clipped_triangles.push_back({ in_v, new_v0, new_v1 });
    }
    else if (inside_points.size() == 2 && outside_points.size() == 1) {
        // Case 3: Two vertices inside, form a quad (two triangles)
        const Vertex& in_v0 = inside_points[0];
        const Vertex& in_v1 = inside_points[1];
        const Vertex& out_v = outside_points[0];
        
        auto find_clip = [&](const Vertex& v) { 
            if (v.worldPos == v0.worldPos) return clip0;
            if (v.worldPos == v1.worldPos) return clip1;
            return clip2;
        };

        glm::vec4 in_c0 = find_clip(in_v0);
        glm::vec4 in_c1 = find_clip(in_v1);
        glm::vec4 out_c = find_clip(out_v);

        // First new vertex
        float t0 = (EPSILON - in_c0.w) / (out_c.w - in_c0.w);
        Vertex new_v0 = interpolate_vertex(in_v0, out_v, t0);
        
        // Second new vertex
        float t1 = (EPSILON - in_c1.w) / (out_c.w - in_c1.w);
        Vertex new_v1 = interpolate_vertex(in_v1, out_v, t1);

        // Create two new triangles to form the quad
        clipped_triangles.push_back({ in_v0, in_v1, new_v0 });
        clipped_triangles.push_back({ in_v1, new_v1, new_v0 });
    }
    // Case 4: (inside_points.size() == 0) -> all outside, do nothing (discard)
}
// v: vertex
// s: point in screen space
// w: world space position
void Renderer::_drawTrianglePhong(
    const Vertex& v0, const Vertex& v1, const Vertex& v2,
    const glm::vec3& s0, const glm::vec3& s1, const glm::vec3& s2,
    const float& w0, const float& w1, const float& w2,
    const std::vector<std::shared_ptr< Light >>& lights, const Camera& camera) {
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
        _drawTrianglePhong(v0, v2, v1, s0, s2, s1, w0, w2, w1, lights, camera);
        return;
    }
    float invW0 = 1.0f / w0;
    float invW1 = 1.0f / w1;
    float invW2 = 1.0f / w2;
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
                // // --- SOLUTION 1: Check Barycentric Coords ---
                // // If the barycentric coordinates have exploded, they are no longer valid.
                // if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(c)) {
                //     continue; // Skip this unstable pixel
                // }

                // Proceed with interpolation
                float z = a * s0.z + b * s1.z + c * s2.z;
                int idx = y * screenWidth + x;

                // // --- SOLUTION 2: Check Interpolated Depth ---
                // // A final check on z before the z-buffer test is the most robust solution.
                // if (!std::isfinite(z)) {
                //     continue; // Skip this unstable pixel
                // }
                
                if (z < zbuffer[idx]) {
                    zbuffer[idx] = z;
                    glm::vec3 pos = v0.worldPos * a + v1.worldPos * b + v2.worldPos * c;
                    glm::vec3 normal = glm::normalize(n0_w * a + n1_w * b + n2_w * c);

                    // 透视修正插值 uv
                    float invW = a * invW0 + b * invW1 + c * invW2;
                    glm::vec2 uv = (a * uv0_w + b * uv1_w + c * uv2_w) / invW;
                    //glm::vec3 color = normal; // Debug：显示 normal，范围应该在 [-1,1]
                    glm::vec3 baseColor = sampleTexture(textureData, uv, textureWidth, textureHeight);
                    glm::vec3 color = _computePhongColor(pos, normal, lights[0], camera.getPosition(), baseColor);
                    // color = normal;
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
    int texX = std::clamp(int(uv.x * texWidth), 0, texWidth - 1);
    int texY = std::clamp(int(uv.y * texHeight), 0, texHeight - 1);
    return Color::Uint32ToVec(textureData[texY * texWidth + texX]); // 纹理采样
}


// Helper to interpolate a full ClippedVertex
ClippedVertex interpolate_clipped_vertex(const ClippedVertex& v0, const ClippedVertex& v1, float t) {
    ClippedVertex result;
    result.vertex = interpolate_vertex(v0.vertex, v1.vertex, t); // Your existing helper
    result.clipPos = lerp(v0.clipPos, v1.clipPos, t);
    return result;
}

// MODIFIED clipping function
void clip_triangle_against_near_plane(
    const ClippedVertex& cv0, const ClippedVertex& cv1, const ClippedVertex& cv2,
    std::vector<std::array<ClippedVertex, 3>>& clipped_tris) 
{
    const ClippedVertex clipped_vertices[] = { cv0, cv1, cv2 };
    std::vector<ClippedVertex> inside_points;
    std::vector<ClippedVertex> outside_points;

    for (int i = 0; i < 3; ++i) {
        if (clipped_vertices[i].clipPos.w >= EPSILON) {
            inside_points.push_back(clipped_vertices[i]);
        } else {
            outside_points.push_back(clipped_vertices[i]);
        }
    }

    if (inside_points.size() == 3) {
        clipped_tris.push_back({ cv0, cv1, cv2 });
    }
    else if (inside_points.size() == 1 && outside_points.size() == 2) {
        const ClippedVertex& in_v = inside_points[0];
        const ClippedVertex& out_v0 = outside_points[0];
        const ClippedVertex& out_v1 = outside_points[1];

        float t0 = (EPSILON - in_v.clipPos.w) / (out_v0.clipPos.w - in_v.clipPos.w);
        ClippedVertex new_v0 = interpolate_clipped_vertex(in_v, out_v0, t0);

        float t1 = (EPSILON - in_v.clipPos.w) / (out_v1.clipPos.w - in_v.clipPos.w);
        ClippedVertex new_v1 = interpolate_clipped_vertex(in_v, out_v1, t1);

        clipped_tris.push_back({ in_v, new_v0, new_v1 });
    }
    else if (inside_points.size() == 2 && outside_points.size() == 1) {
        const ClippedVertex& in_v0 = inside_points[0];
        const ClippedVertex& in_v1 = inside_points[1];
        const ClippedVertex& out_v = outside_points[0];

        float t0 = (EPSILON - in_v0.clipPos.w) / (out_v.clipPos.w - in_v0.clipPos.w);
        ClippedVertex new_v0 = interpolate_clipped_vertex(in_v0, out_v, t0);

        float t1 = (EPSILON - in_v1.clipPos.w) / (out_v.clipPos.w - in_v1.clipPos.w);
        ClippedVertex new_v1 = interpolate_clipped_vertex(in_v1, out_v, t1);

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
        const glm::mat4 modelMatrix = object.getMatrix();
        const glm::mat4 mvp = viewProjectionMatrix * modelMatrix;
        
        const Mesh& mesh = object.getMesh();
        const std::vector<Vertex>& vertices = mesh.vertices;
        const std::vector<unsigned int>& indices = mesh.indices;

        #pragma omp parallel for
        for (size_t i = 0; i < indices.size(); i += 3) {
            // Package original vertices with their clip-space positions
            ClippedVertex cv0 = { vertices[indices[i]],   mvp * glm::vec4(vertices[indices[i]].worldPos, 1.0f) };
            ClippedVertex cv1 = { vertices[indices[i+1]], mvp * glm::vec4(vertices[indices[i+1]].worldPos, 1.0f) };
            ClippedVertex cv2 = { vertices[indices[i+2]], mvp * glm::vec4(vertices[indices[i+2]].worldPos, 1.0f) };

            // Optional: Back-face culling on original vertices
            if (_isBackFacingViewSpace(cv0.vertex.worldPos, cv1.vertex.worldPos, cv2.vertex.worldPos, scene.camera.getPosition())) {
                continue;
            }

            // Perform clipping
            std::vector<std::array<ClippedVertex, 3>> clipped_triangles;
            clip_triangle_against_near_plane(cv0, cv1, cv2, clipped_triangles);

            // Process each resulting (clipped) triangle
            for (const auto& tri : clipped_triangles) {
                const ClippedVertex& new_cv0 = tri[0];
                const ClippedVertex& new_cv1 = tri[1];
                const ClippedVertex& new_cv2 = tri[2];

                // **NO RE-PROJECTION**. Use the interpolated clip coordinates directly.
                glm::vec4 clip0 = new_cv0.clipPos;
                glm::vec4 clip1 = new_cv1.clipPos;
                glm::vec4 clip2 = new_cv2.clipPos;

                // By definition, the w-component of any new vertex will now be exactly EPSILON,
                // and the w of original vertices will be unchanged. This prevents division by zero.

                // Perform perspective divide and viewport transform
                glm::vec3 ndc0 = glm::vec3(clip0) / clip0.w;
                glm::vec3 ndc1 = glm::vec3(clip1) / clip1.w;
                glm::vec3 ndc2 = glm::vec3(clip2) / clip2.w;

                glm::vec3 s0 = { (ndc0.x + 1.0f) * 0.5f * screenWidth, (1.0f - ndc0.y) * 0.5f * screenHeight, (ndc0.z + 1.0f) * 0.5f };
                glm::vec3 s1 = { (ndc1.x + 1.0f) * 0.5f * screenWidth, (1.0f - ndc1.y) * 0.5f * screenHeight, (ndc1.z + 1.0f) * 0.5f };
                glm::vec3 s2 = { (ndc2.x + 1.0f) * 0.5f * screenWidth, (1.0f - ndc2.y) * 0.5f * screenHeight, (ndc2.z + 1.0f) * 0.5f };
                _drawTrianglePhong(
                    new_cv0.vertex, new_cv1.vertex, new_cv2.vertex,
                    s0, s1, s2,
                    clip0.w, clip1.w, clip2.w, // Use the correct w values
                    scene.lights, scene.camera
                );

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
            // printf("Rendering pixel (%d, %d)\n", x, y);
            // printf("Ray direction: (%f, %f, %f)\n", ray.direction.x, ray.direction.y, ray.direction.z);
            // Trace the ray through the scene
            glm::vec3 color = traceRay(ray, scene, 0);
            // Write the color to the framebuffer
            framebuffer.setPixel(x, y, Color::VecToUint32(color));
        }
    }
}

glm::vec3 Renderer::traceRay(const Ray& ray, const Scene& scene, int depth) {
    if (depth > MAX_DEPTH) {
        return glm::vec3(0.0f); // Terminate recursion
    }

    // Find the closest intersection
    Intersection intersection;
    if (!scene.intersect(ray, intersection)) {
        return scene.getBackgroundColor(); // No intersection, return background grey color
    }
    // Compute local shading (e.g., Phong shading)
    glm::vec3 localColor = computeLocalShading(intersection, scene);
    // // Compute reflection
    // glm::vec3 reflectionColor(0.0f);
    // if (intersection.material->reflectivity > 0.0f) {
    //     Ray reflectedRay = computeReflectedRay(ray, intersection);
    //     reflectionColor = traceRay(reflectedRay, scene, depth + 1) * intersection.material.reflectivity;
    // }

    // // Compute refraction
    // glm::vec3 refractionColor(0.0f);
    // if (intersection.material->transparency > 0.0f) {
    //     Ray refractedRay = computeRefractedRay(ray, intersection);
    //     refractionColor = traceRay(refractedRay, scene, depth + 1) * intersection.material.transparency;
    // }

    // Combine local shading, reflection, and refraction
    // return localColor + reflectionColor + refractionColor;
    return localColor;
}

glm::vec3 Renderer::computeLocalShading(const Intersection& isect, const Scene& scene) {
    glm::vec3 color(0.0f);
    glm::vec3 viewDir = glm::normalize(scene.camera.getPosition() - isect.position);
    const auto& mat = *isect.material;

    // 常量项：环境光
    const float ambientStrength = 0.1f;
    glm::vec3 ambient = ambientStrength * mat.diffuseColor;
    color += ambient;

    // 对每个点光源进行光照计算
    for (const auto& light : scene.lights) {
        glm::vec3 lightPos = light->getDirection(glm::vec3(0.0f)); // Replace with a method that provides the light's position or direction
        float lightIntensity = light->getIntensity(isect.position);
        glm::vec3 lightColor = light->getColor() * lightIntensity;
        

        glm::vec3 lightDir = glm::normalize(lightPos - isect.position);
        float diff = glm::max(glm::dot(isect.normal, lightDir), 0.0f);
        float shadowFactor = computeSoftShadow(isect.position, scene, lightPos, 4); // 16 次采样
        glm::vec3 diffuse = diff * mat.diffuseColor * lightColor * shadowFactor;
        color += diffuse;

        if (glm::dot(isect.normal, lightDir) > 0.0f) { // 仅在法线与光源方向一致时计算高光
            glm::vec3 halfwayDir = glm::normalize(lightDir + viewDir);
            float spec = glm::pow(glm::max(glm::dot(isect.normal, halfwayDir), 0.0f), mat.shininess);
            glm::vec3 specular = spec * mat.specularColor * lightColor;
            color += specular; // 累加高光
        }
    }

    return glm::clamp(color, 0.0f, 1.0f); // 保证颜色在 [0,1]
}

bool Renderer::isInShadow(const glm::vec3& point, const Scene& scene, const glm::vec3& lightPos) {
    glm::vec3 shadowRayDir = glm::normalize(lightPos - point);
    Ray shadowRay(point + shadowRayDir * 0.001f, shadowRayDir); // 偏移避免自遮挡
    Intersection shadowIsect;
    shadowIsect.t = glm::length(lightPos - point);

    return scene.intersect(shadowRay, shadowIsect); // 如果有遮挡，返回 true
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
    textureWidth = 64;
    textureHeight = 64;
    textureData = ResourceLoader::loadTextureFromFile("Resources\\checker.jpg", textureWidth, textureHeight);
    // for (int i = 0; i < 10; ++i) {
    //     uint32_t pixel = textureData[i];
    //     SDL_Log("Pixel %d: R=%d, G=%d, B=%d, A=%d",
    //             i,
    //             (pixel & 0xFF),         // Red
    //             (pixel >> 8) & 0xFF,   // Green
    //             (pixel >> 16) & 0xFF,  // Blue
    //             (pixel >> 24) & 0xFF); // Alpha
    // }
}
