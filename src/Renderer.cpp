#include "Renderer.h"

#include "ResourceLoader.h"
#include "Scene.h"

//#include "Debug.h"
//#define DEBUG_MODE

void Renderer::clearBuffers() {
	framebuffer.clear(0xFF000000);
	zbuffer.clear(std::numeric_limits<float>::max());  // Clear depth buffer to max depth
}

bool Renderer::_isBackFacing(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
	glm::vec3 e1 = v1 - v0;
	glm::vec3 e2 = v2 - v0;
	float z = e1.x * e2.y - e1.y * e2.x;
	return z > 0;  // ????
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

// viewport transform: from NDC to window
glm::vec4 Renderer::_ProjectToScreen(const glm::vec3& v, const glm::mat4& mvp, int w, int h) {
    const glm::vec4 v4(v.x, v.y, v.z, 1.0f);
    // std::cout << "mvp matrix: " << mvp << std::endl;
    glm::vec4 clip = mvp * v4; // 列主序乘法
    // std::cout << "clip: " << clip.x << ", " << clip.y << ", " << clip.z << ", " << clip.w << std::endl;
    if (fabs(clip.w) < EPSILON) {
        clip.w = EPSILON * (clip.w < 0 ? -1.0f : 1.0f); // 保留符号
    }

    glm::vec4 ndc = clip / clip.w;
    ndc.z = (ndc.z + 1.0f) * 0.5f;
    float x = (ndc.x + 1.0f) * 0.5f * w;
    float y = (1.0f - ndc.y) * 0.5f * h; // Y 方向反转
    return glm::vec4(x, y, ndc.z, clip.w);
}

glm::vec4 Renderer::_screenToClip(const glm::vec4& s, int screenWidth, int screenHeight){
    float x = s.x / (0.5f * screenWidth) - 1.0f;
    float y = 1.0f - s.y / (0.5f * screenHeight);
    float z = s.z * 2.0f - 1.0f;
    return glm::vec4(x, y, z, s.w);
}
std::vector<Renderer::ClipVertex> Renderer::clipTriangle(const ClipVertex& v0, const ClipVertex& v1, const ClipVertex& v2) {
    std::vector<ClipVertex> input = { v0, v1, v2 };

    // 六个平面：x/w <= 1, x/w >= -1, ...
    for (int plane = 0; plane < 6; ++plane) {
        std::vector<ClipVertex> output;
        for (size_t i = 0; i < input.size(); ++i) {
            const ClipVertex& current = input[i];
            const ClipVertex& prev = input[(i + input.size() - 1) % input.size()];
            bool currInside = inside(current.clipPos, plane);
            bool prevInside = inside(prev.clipPos, plane);

            if (currInside != prevInside) {
                output.push_back(intersect(prev, current, plane));
            }
            if (currInside) {
                output.push_back(current);
            }
        }
        input = output;
        if (input.empty()) return {}; // 全部裁掉
    }
    return input;
}
bool Renderer::inside(const glm::vec4& v, int plane) {
    switch (plane) {
        case 0: return v.x >= -v.w; // LEFT
        case 1: return v.x <=  v.w; // RIGHT
        case 2: return v.y >= -v.w; // BOTTOM
        case 3: return v.y <=  v.w; // TOP
        case 4: return v.z >= -v.w; // NEAR
        case 5: return v.z <=  v.w; // FAR
        default: return true;
    }
}
Renderer::ClipVertex Renderer::intersect(const ClipVertex& v1, const ClipVertex& v2, int plane) {
    float t = computeT(v1.clipPos, v2.clipPos, plane);
    Vertex out;
    glm::vec4 out_clipPos = glm::mix(v1.clipPos, v2.clipPos, t);
    out.worldPos = glm::mix(v1.vertex.worldPos, v2.vertex.worldPos, t);
    out.normal = glm::normalize(glm::mix(v1.vertex.normal, v2.vertex.normal, t));
    out.uv = glm::mix(v1.vertex.uv, v2.vertex.uv, t);
    float out_invW = glm::mix(v1.invW, v2.invW, t);
    return ClipVertex{ out, out_clipPos, out_invW };
}
float Renderer::computeT(const glm::vec4& v1, const glm::vec4& v2, int plane) {
    float a, b;
    switch (plane) {
        case 0: a = v1.x + v1.w; b = v2.x + v2.w; break;
        case 1: a = v1.w - v1.x; b = v2.w - v2.x; break;
        case 2: a = v1.y + v1.w; b = v2.y + v2.w; break;
        case 3: a = v1.w - v1.y; b = v2.w - v2.y; break;
        case 4: a = v1.z + v1.w; b = v2.z + v2.w; break;
        case 5: a = v1.w - v1.z; b = v2.w - v2.z; break;
        default: return 0.0f;
    }
    return a / (a - b);
}

// v: vertex
// s: point in screen space
// w: world space position
void Renderer::_drawTrianglePhong(
    const Vertex& v0, const Vertex& v1, const Vertex& v2,
    const glm::vec3& s0, const glm::vec3& s1, const glm::vec3& s2,
    const float& w0, const float& w1, const float& w2,
    const std::vector<std::shared_ptr< Light >>& lights, const Camera& camera) {
    if (w0 < EPSILON || w1 < EPSILON || w2 < EPSILON) {
        // printf("skipped point: w0=%.2f, w1=%.2f, w2=%.2f\n", w0, w1, w2);
        // printf("world position: v0=(%.2f, %.2f, %.2f), v1=(%.2f, %.2f, %.2f), v2=(%.2f, %.2f, %.2f)\n", v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
        // printf("screen position: s0=(%.2f, %.2f, %.2f), s1=(%.2f, %.2f, %.2f), s2=(%.2f, %.2f, %.2f)\n", s0.x, s0.y, s0.z, s1.x, s1.y, s1.z, s2.x, s2.y, s2.z);
        return; // 避免除以零
    }

    // 保证顺时针方向，防止 area 负值带来插值错误
    float area = glm::cross(s1 - s0, s2 - s0).z;
    if (fabs(area) < EPSILON) {
        printf("Degenerate triangle: area=%.2f\n", area);
        printf("v0=(%.2f, %.2f, %.2f), v1=(%.2f, %.2f, %.2f), v2=(%.2f, %.2f, %.2f)\n", v0.worldPos.x, v0.worldPos.y, v0.worldPos.z, v1.worldPos.x, v1.worldPos.y, v1.worldPos.z, v2.worldPos.x, v2.worldPos.y, v2.worldPos.z);
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
                float z = a * s0.z + b * s1.z + c * s2.z;
                int idx = y * screenWidth + x;
                
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

void Renderer::render(Scene scene) {
	// repaint
	clearBuffers();
    scene.camera.setAspect(static_cast<float>(screenWidth) / screenHeight);
	glm::mat4 projectionMatrix = scene.camera.getProjectionMatrix();
	glm::mat4 viewMatrix = scene.camera.getViewMatrix();
    
    // 计算 MVP 矩阵
    glm::mat4 vp = projectionMatrix * viewMatrix; // 注意这里是列主序矩阵乘法
	for (Object& object : scene.objects) {
		const glm::mat4 modelMatrix = object.getMatrix();
		glm::mat4 mvp = vp * modelMatrix; 

		const Mesh& mesh = object.getMesh();
		const std::vector<Vertex>& vertices = mesh.vertices;
		const std::vector<unsigned int>& indices = mesh.indices;
        
        // screen positions + w
        std::vector<glm::vec3> sPos(vertices.size());
        std::vector<float> sW(vertices.size());
        for (size_t i = 0; i < vertices.size(); i++) {
            glm::vec4 s = _ProjectToScreen(vertices[i].worldPos, mvp, screenWidth, screenHeight);
            sPos[i] = glm::vec3(s.x, s.y, s.z);
            sW[i] = s.w;
            //printf("(%f) ", sPos[i].z);
        }

        for (size_t i = 0; i < indices.size(); i += 3) {
            if (_isBackFacing(sPos[indices[i]], sPos[indices[i + 1]], sPos[indices[i + 2]])) continue;
            _drawTrianglePhong(
                vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]], 
                sPos[indices[i]], sPos[indices[i + 1]], sPos[indices[i + 2]], // screen positions
                sW[indices[i]], sW[indices[i + 1]], sW[indices[i + 2]], // w values
                scene.lights, scene.camera
            );
        }
    }
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
