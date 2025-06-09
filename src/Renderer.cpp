#include "Renderer.h"

#include "Scene.h"
//#include "Debug.h"
//#define DEBUG_MODE

void Renderer::_drawTriangleFilled(Vec3 v0, Vec3 v1, Vec3 v2, uint32_t color, int w, int h) {
	#ifdef DEBUG_MODE
	std::cout << "Triangle: v0: " << v0.x << ", " << v0.y << ", " << v0.z << std::endl;
	std::cout << "Triangle: v1: " << v1.x << ", " << v1.y << ", " << v1.z << std::endl;
	std::cout << "Triangle: v2: " << v2.x << ", " << v2.y << ", " << v2.z << std::endl;
	#endif

	if (v0.y > v1.y) std::swap(v0, v1);
	if (v0.y > v2.y) std::swap(v0, v2);
	if (v1.y > v2.y) std::swap(v1, v2);

	auto edgeInterp = [](const Vec3& a, const Vec3& b, float y) -> Vec3 {
		float t = (y - a.y) / (b.y - a.y);
		return a + (b - a) * t;
	};

	auto drawScanline = [&](int y, Vec3 left, Vec3 right) {
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
	Vec3 vi = edgeInterp(v0, v2, v1.y);

	// �ϰ벿�֣�v0, v1, vi��
	for (int y = int(v0.y); y <= int(v1.y); ++y) {
		Vec3 a = edgeInterp(v0, v1, float(y));
		Vec3 b = edgeInterp(v0, v2, float(y));
		drawScanline(y, a, b);
	}

	// �°벿�֣�v1, vi, v2��
	for (int y = int(v1.y); y <= int(v2.y); ++y) {
		Vec3 a = edgeInterp(v1, v2, float(y));
		Vec3 b = edgeInterp(vi, v2, float(y));
		drawScanline(y, a, b);
	}
}

// v: vertex
// s: point in screen space
// w: world space position
void Renderer::_drawTrianglePhong(
    const Vertex& v0_, const Vertex& v1_, const Vertex& v2_,
    const Vec3& w0_, const Vec3& w1_, const Vec3& w2_,
    const Vec3& s0_, const Vec3& s1_, const Vec3& s2_,
    const std::vector<std::shared_ptr<Light>>& lights, const Camera& camera, const Vec3& baseColor) {

    // 剔除 ProjectToScreen 返回的无效点
    if (s0_.x >= FLT_MAX || s1_.x >= FLT_MAX || s2_.x >= FLT_MAX) return;

    // 保证顺时针方向，防止 area 负值带来插值错误
    Vec3 s0 = s0_, s1 = s1_, s2 = s2_;
    Vec3 w0 = w0_, w1 = w1_, w2 = w2_;
    Vertex v0 = v0_, v1 = v1_, v2 = v2_;
    float area = (s1 - s0).cross(s2 - s0).z;
    if (fabs(area) < EPSILON) return; // 退化三角形
    if (area < 0.0f) {
        std::swap(s1, s2);
        std::swap(w1, w2);
        std::swap(v1, v2);
        area = -area;
    }

    int minX = std::max(0, (int)std::floor(std::min({ s0.x, s1.x, s2.x })));
    int maxX = std::min(screenWidth - 1, (int)std::ceil(std::max({ s0.x, s1.x, s2.x })));
    int minY = std::max(0, (int)std::floor(std::min({ s0.y, s1.y, s2.y })));
    int maxY = std::min(screenHeight - 1, (int)std::ceil(std::max({ s0.y, s1.y, s2.y })));

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            Vec3 p(x + 0.5f, y + 0.5f, 0.0f);
            if (insideTriangle(p, s0, s1, s2)) {
                float a = ((s1 - p).cross(s2 - p)).z / area;
                float b = ((s2 - p).cross(s0 - p)).z / area;
                float c = ((s0 - p).cross(s1 - p)).z / area;
                float z = a * s0.z + b * s1.z + c * s2.z;
                int idx = y * screenWidth + x;
                if (z < zbuffer[idx]) {
                    zbuffer[idx] = z;
                    Vec3 pos = w0 * a + w1 * b + w2 * c;
                    Vec3 normal = (v0.normal * a + v1.normal * b + v2.normal * c).normalized();
                    Vec3 color = normal; // Debug：显示 normal，范围应该在 [-1,1]
                    //Vec3 color = _computePhongColor(pos, normal, lights[0], camera.getPosition(), baseColor);

                    framebuffer[idx] = Color::VecToUint32((color + Vec3(1.0f)) * 0.5f); // 映射 [-1,1] → [0,1]
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
Vec3 Renderer::_ProjectToScreen(const Vec3& v, const Mat4& mvp, int w, int h) {
    const Vec4 v4(v.x, v.y, v.z, 1.0f);
    Vec4 clip = v4 * mvp; // 行主序乘法
    if (clip.w < EPSILON) return Vec3(FLT_MAX, FLT_MAX, FLT_MAX); // 剔除不可见点

    Vec4 ndc = clip / clip.w;
    float x = (ndc.x + 1.0f) * 0.5f * w;
    float y = (1.0f - ndc.y) * 0.5f * h; // Y 方向反转
    return Vec3(x, y, ndc.z);
}

void Renderer::render(Scene scene) {
	// repaint
	clearBuffers();
    scene.camera.setAspect(static_cast<float>(screenWidth) / screenHeight);
	Mat4 projectionMatrix = scene.camera.getProjectionMatrix();
	Mat4 vp;
    switch (scene.camera.projectionType) {
        case Camera::PERSPECTIVE:
		    vp = scene.camera.getViewMatrix() * projectionMatrix;
            break;
        case Camera::ORTHOGRAPHIC:
            vp = projectionMatrix;
            break;
    }
	
	for (Object& object : scene.objects) {
		const Mat4 modelMatrix = object.getMatrix();
		const Mat4 mvp = modelMatrix * vp;

		const Mesh& mesh = object.getMesh();
		const std::vector<Vertex>& vertices = mesh.vertices;
		const std::vector<unsigned int>& indices = mesh.indices;
		
		// world positions
		std::vector<Vec3> wPos(vertices.size());
		for (size_t i = 0; i < vertices.size(); i++) {
			Vec4 w4 =  Vec4(vertices[i].position, 1.0f) * modelMatrix;
			wPos[i] = Vec3(w4.x, w4.y, w4.z);
		}
        // screen positions
		std::vector<Vec3> sPos(vertices.size());
		for (size_t i = 0; i < vertices.size(); i++) {
			sPos[i] = _ProjectToScreen(vertices[i].position, mvp, screenWidth, screenHeight);
		}

		Vec3 baseColor = Vec3(0.2f, 0.4f, 0.6f);
		for (size_t i = 0; i < indices.size(); i += 3) {

			if (_isBackFacing(sPos[indices[i]], sPos[indices[i + 1]], sPos[indices[i + 2]])) continue;
			// _drawTriangleFilled(vertices[indices[i]].position, vertices[indices[i + 1]].position, vertices[indices[i + 2]].position, 0xFFFFFF, screenWidth, screenHeight);
			_drawTrianglePhong(
				vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]],  // vertices (local position, normal, uv)
				wPos[indices[i]], wPos[indices[i + 1]], wPos[indices[i + 2]], // world positions
				sPos[indices[i]], sPos[indices[i + 1]], sPos[indices[i + 2]], // screen positions
				scene.lights, scene.camera, baseColor
			);
		}
	}
}

bool Renderer::_isBackFacing(const Vec3& v0, const Vec3& v1, const Vec3& v2) {
	Vec3 e1 = v1 - v0;
	Vec3 e2 = v2 - v0;
	float z = e1.x * e2.y - e1.y * e2.x;
	return z > 0;  // ????
}



Vec3 Renderer::_computePhongColor(const Vec3& pos, const Vec3& normal, const std::shared_ptr<Light>& light, const Vec3& cameraPos, const Vec3& baseColor)
{
	Vec3 L = light->getDirection(pos);
	Vec3 N = normal.normalized();
	Vec3 V = (cameraPos - pos).normalized();
	Vec3 R = (2.0f * N.dot(L) * N - L).normalized();

	float diff = std::max(0.0f, N.dot(L));
	float spec = std::pow(std::max(0.0f, R.dot(V)), 16.0f); // shininess

	Vec3 ambient = 0.1f * baseColor;
	Vec3 diffuse = diff * baseColor;
	Vec3 specular = spec * Vec3{ 1,1,1 }; // white specular

	Vec3 color = (ambient + diffuse + specular)*light->getIntensity(pos);
	return color.clamp(0.0f, 1.0f);
}
