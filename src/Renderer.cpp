#include "Renderer.h"

#include "ResourceLoader.h"
#include "Scene.h"
//#include "Debug.h"
//#define DEBUG_MODE

void Renderer::_drawTriangleFilled(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, uint32_t color, int w, int h) {
	#ifdef DEBUG_MODE
	std::cout << "Triangle: v0: " << v0.x << ", " << v0.y << ", " << v0.z << std::endl;
	std::cout << "Triangle: v1: " << v1.x << ", " << v1.y << ", " << v1.z << std::endl;
	std::cout << "Triangle: v2: " << v2.x << ", " << v2.y << ", " << v2.z << std::endl;
	#endif

	if (v0.y > v1.y) std::swap(v0, v1);
	if (v0.y > v2.y) std::swap(v0, v2);
	if (v1.y > v2.y) std::swap(v1, v2);

	auto edgeInterp = [](const glm::vec3& a, const glm::vec3& b, float y) -> glm::vec3 {
		float t = (y - a.y) / (b.y - a.y);
		return a + (b - a) * t;
	};

	auto drawScanline = [&](int y, glm::vec3 left, glm::vec3 right) {
		if (y < 0 || y >= h) return;
		if (left.x > right.x) std::swap(left, right);
		int xStart = std::max(int(left.x), 0);
		int xEnd = std::min(int(right.x), w - 1);

		for (int x = xStart; x <= xEnd; ++x) {
			float t = (x - left.x) / (right.x - left.x + 1e-5f);
			float z = left.z + t * (right.z - left.z);

			int idx = y * w + x;
			if (z < zbuffer[idx]) {
				zbuffer[idx] = z;
				framebuffer[idx] = color;
			}
		}
	};

	// Middle point on long edge between v0 and v2 at y = v1.y
	glm::vec3 vi = edgeInterp(v0, v2, v1.y);

	for (int y = int(v0.y); y <= int(v1.y); ++y) {
		glm::vec3 a = edgeInterp(v0, v1, float(y));
		glm::vec3 b = edgeInterp(v0, v2, float(y));
		drawScanline(y, a, b);
	}

	for (int y = int(v1.y); y <= int(v2.y); ++y) {
		glm::vec3 a = edgeInterp(v1, v2, float(y));
		glm::vec3 b = edgeInterp(vi, v2, float(y));
		drawScanline(y, a, b);
	}
}

// v: vertex
// s: point in screen space
// w: world space position
void Renderer::_drawTrianglePhong(
    const glm::vec3& w0_, const glm::vec3& w1_, const glm::vec3& w2_,
    const glm::vec3& s0_, const glm::vec3& s1_, const glm::vec3& s2_,
    glm::vec3 n0, glm::vec3 n1, glm::vec3 n2,
    glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2,
    const std::vector<std::shared_ptr<Light>>& lights, const Camera& camera) {

    // 剔除 ProjectToScreen 返回的无效点
    if (s0_.x >= FLT_MAX || s1_.x >= FLT_MAX || s2_.x >= FLT_MAX) return;

    // 保证顺时针方向，防止 area 负值带来插值错误
    glm::vec3 s0 = s0_, s1 = s1_, s2 = s2_;
    glm::vec3 w0 = w0_, w1 = w1_, w2 = w2_;
    float area = glm::cross(s1 - s0, s2 - s0).z;
    if (fabs(area) < EPSILON) return; // 退化三角形
    if (area < 0.0f) {
        std::swap(s1, s2);
        std::swap(w1, w2);
        std::swap(n1, n2);
        std::swap(uv1, uv2);
        area = -area;
    }

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
                    glm::vec3 pos = w0 * a + w1 * b + w2 * c;
                    glm::vec3 normal = glm::normalize(n0 * a + n1 * b + n2 * c);
                    glm::vec2 uv = uv0 * a + uv1 * b + uv2 * c;
                    // glm::vec2 uv = glm::vec2(x, y);
                    //glm::vec3 color = normal; // Debug：显示 normal，范围应该在 [-1,1]
                    glm::vec3 baseColor = sampleTexture(textureData, uv, textureWidth, textureHeight);
                    //printf("Pixel baseColor=(%.2f, %.2f, %.2f)\n", baseColor.x, baseColor.y, baseColor.z);
                    glm::vec3 color = _computePhongColor(pos, normal, lights[0], camera.getPosition(), baseColor);
                    // glm::vec3 color = baseColor;
                    framebuffer[idx] = Color::VecToUint32(color); // 映射 [-1,1] → [0,1]
                }
            }
        }
    }
}

void Renderer::clearBuffers() {
	framebuffer.clear(0xFF000000);
	zbuffer.clear(std::numeric_limits<float>::max());  // Clear depth buffer to max depth
}

// viewport transform: from NDC to window
glm::vec3 Renderer::_ProjectToScreen(const glm::vec3& v, const glm::mat4& mvp, int w, int h) {
    const glm::vec4 v4(v.x, v.y, v.z, 1.0f);
    // std::cout << "mvp matrix: " << mvp << std::endl;
    glm::vec4 clip = mvp * v4; // 列主序乘法
    // std::cout << "clip: " << clip.x << ", " << clip.y << ", " << clip.z << ", " << clip.w << std::endl;
    if (clip.w < EPSILON) {
        //std::cout << "Invalid clip.w: " << clip.w << std::endl;
        return glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    }


    glm::vec4 ndc = clip / clip.w;
    ndc.z = (ndc.z + 1.0f) * 0.5f; // 归一化设备坐标 NDC [-1, 1] → [0, 1]
    float x = (ndc.x + 1.0f) * 0.5f * w;
    float y = (1.0f - ndc.y) * 0.5f * h; // Y 方向反转
    return glm::vec3(x, y, ndc.z);
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
		
		// world positions
		std::vector<glm::vec3> wPos(vertices.size());
		for (size_t i = 0; i < vertices.size(); i++) {
			glm::vec4 w4 = modelMatrix * glm::vec4(vertices[i].position.x, vertices[i].position.y, vertices[i].position.z, 1.0f);
			wPos[i] = glm::vec3(w4.x, w4.y, w4.z);
		}
        // screen positions
		std::vector<glm::vec3> sPos(vertices.size());
		for (size_t i = 0; i < vertices.size(); i++) {
			sPos[i] = _ProjectToScreen(vertices[i].position, mvp, screenWidth, screenHeight);
            //printf("(%f) ", sPos[i].z);
		}

        // normal projection
        std::vector<glm::vec3> vNormals(vertices.size());
        for (size_t i = 0; i < vertices.size(); i++) {
            vNormals[i] = glm::vec3(modelMatrix * glm::vec4(vertices[i].normal, 0.0f)); // 只需要旋转，不需要平移
        }


        for (size_t i = 0; i < indices.size(); i += 3) {
            if (_isBackFacing(sPos[indices[i]], sPos[indices[i + 1]], sPos[indices[i + 2]])) continue;
            // _drawTriangleFilled(vertices[indices[i]].position, vertices[indices[i + 1]].position, vertices[indices[i + 2]].position, 0xFFFFFF, screenWidth, screenHeight);
            _drawTrianglePhong(
                wPos[indices[i]], wPos[indices[i + 1]], wPos[indices[i + 2]], // world positions
                sPos[indices[i]], sPos[indices[i + 1]], sPos[indices[i + 2]], // screen positions
                vNormals[indices[i]], vNormals[indices[i + 1]], vNormals[indices[i + 2]], // normals
                vertices[indices[i]].uv, vertices[indices[i + 1]].uv, vertices[indices[i + 2]].uv, // texture coordinates
                scene.lights, scene.camera
            );
        }
    }
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

Renderer::Renderer(int width, int height){
    screenWidth = width;
    screenHeight = height;
    framebuffer = Buffer<uint32_t>(width, height);
    zbuffer = Buffer<float>(width, height);
    clearBuffers();
    textureWidth = 256;
    textureHeight = 256;
    textureData = ResourceLoader::loadTextureFromFile("Resources\\tex.jpg", textureWidth, textureHeight);
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
