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
	// 排序y（v0.y <= v1.y <= v2.y）
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
		int xStart = max(int(left.x), 0);
		int xEnd = min(int(right.x), w - 1);

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

	// 上半部分（v0, v1, vi）
	for (int y = int(v0.y); y <= int(v1.y); ++y) {
		Vec3 a = edgeInterp(v0, v1, float(y));
		Vec3 b = edgeInterp(v0, v2, float(y));
		drawScanline(y, a, b);
	}

	// 下半部分（v1, vi, v2）
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
	const Vertex& v0, const Vertex& v1, const Vertex& v2,
	const Vec3& w0, const Vec3& w1, const Vec3& w2,
	const Vec3& s0, const Vec3& s1, const Vec3& s2,
	const std::vector<std::shared_ptr< Light >> &light, const Camera& camera, const Vec3& baseColor) {

	// Calculate bounding box
	int minX = (std::max)(0, (int)std::floor((std::min)({ s0.x, s1.x, s2.x })));
	int maxX = (std::min)(screenWidth - 1, (int)std::ceil((std::max)({ s0.x, s1.x, s2.x })));
	int minY = (std::max)(0, (int)std::floor((std::min)({ s0.y, s1.y, s2.y })));
	int maxY = (std::min)(screenHeight - 1, (int)std::ceil((std::max)({ s0.y, s1.y, s2.y })));

	float area = (s1 - s0).cross(s2 - s0).z;
	if (-EPSILON < area && area < EPSILON) return; // Degenerate triangle

	for (int y = minY; y <= maxY; ++y) {
		for (int x = minX; x <= maxX; ++x) {
			Vec3 p(x + 0.5f, y + 0.5f, 0.0);
			if (insideTriangle(p, s0, s1, s2)) {
				float a = ((s1 - p).cross(s2 - p)).z / area;
				float b = ((s2 - p).cross(s0 - p)).z / area;
				float c = ((s0 - p).cross(s1 - p)).z / area;
				float z = a * w0.z + b * w1.z + c * w2.z;  // p's z value of world position
				int idx = y * screenWidth + x;
				if (z < zbuffer[idx]) {
					zbuffer[idx] = z;
					Vec3 pos = a * w0 + b * w1 + c * w2; // p's world position
					Vec3 normal = a * v0.normal + b * v1.normal + c * v2.normal; // p's normal
					Vec2 uv = a * v0.uv + b * v1.uv + c * v2.uv; // p's uv
					Vec3 color = _computePhongColor(pos, normal, light[0], camera.getPosition(), baseColor);
					framebuffer[idx] = Color::VecToUint32(color);
				}
			}
		}
	}
}

void Renderer::clearBuffers() {
	framebuffer.clear(0xFF000000); // Clear to black
	zbuffer.clear(1.0f); // Clear depth buffer to max depth
}

// viewport transform: from NDC to window
Vec3 Renderer::_ProjectToScreen(const Vec3& v, const Mat4& mvp, int w, int h) {
	// local -> world -> view -> clip
	Vec4 p = mvp * Vec4(v, 1.0f);

	// clip -> NDC
	if (p.w < EPSILON) return Vec3{ 0, 0, 0 }; // 防止除以0
	else p /= p.w;
	// NDC -> window
	return Vec3{
		(p.x * 0.5f + 0.5f) * w,
		(1.0f - (p.y * 0.5f + 0.5f)) * h, // y 反向映射到窗口
		p.z
	};
}

void Renderer::render(HDC hdc, Scene scene) {
	// repaint
	clearBuffers();

	Mat4 projectionMatrix = scene.camera.getProjectionMatrix();
	Mat4 vp;
	if (scene.camera.projectionType == Camera::PERSPECTIVE) {
		Mat4 viewMatrix = scene.camera.getViewMatrix();
		vp = projectionMatrix * viewMatrix;
	} 
	else if (scene.camera.projectionType == Camera::ORTHOGRAPHIC) {
		scene.camera.setAspect(static_cast<float>(screenWidth) / screenHeight);
		vp = projectionMatrix;
	}
	
	for (Object& object : scene.objects) {
		Mat4 modelMatrix = object.getMatrix();
		Mat4 mvp = vp * modelMatrix;

		const Mesh& mesh = object.getMesh();
		const std::vector<Vertex>& vertices = mesh.vertices;
		const std::vector<unsigned int>& indices = mesh.indices;
		
		// world positions
		std::vector<Vec3> wPos(vertices.size());
		for (size_t i = 0; i < vertices.size(); i++) {
			Vec4 w4 = modelMatrix * Vec4(vertices[i].position, 1.0f);
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
			//_drawTriangleFilled(v0, v1, v2, 0xFFFFFF, screenWidth, screenHeight);
			_drawTrianglePhong(
				vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]],  // vertices (local position, normal, uv)
				wPos[indices[i]], wPos[indices[i + 1]], wPos[indices[i + 2]], // world positions
				sPos[indices[i]], sPos[indices[i + 1]], sPos[indices[i + 2]], // screen positions
				scene.lights, scene.camera, baseColor
			);
		}
	}
	_presentToHDC(hdc, screenWidth, screenHeight);
}

bool Renderer::_isBackFacing(const Vec3& v0, const Vec3& v1, const Vec3& v2) {
	Vec3 e1 = v1 - v0;
	Vec3 e2 = v2 - v0;
	float z = e1.x * e2.y - e1.y * e2.x;
	return z < 0;
}

void Renderer::_presentToHDC(HDC hdc, int w, int h) {
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = -h; // top-down
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	StretchDIBits(
		hdc,
		0, 0, w, h,
		0, 0, w, h,
		framebuffer.data(),
		&bmi,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

Vec3 Renderer::_computePhongColor(const Vec3& pos, const Vec3& normal, const std::shared_ptr<Light>& light, const Vec3& cameraPos, const Vec3& baseColor)
{
	Vec3 L = light->getDirection(pos);
	Vec3 N = normal.normalized();
	Vec3 V = (cameraPos - pos).normalized();
	Vec3 R = (2.0f * N.dot(L) * N - L).normalized();

	float diff = max(0.0f, N.dot(L));
	float spec = std::pow(max(0.0f, R.dot(V)), 16.0f); // shininess

	Vec3 ambient = 0.1f * baseColor;
	Vec3 diffuse = diff * baseColor;
	Vec3 specular = spec * Vec3{ 1,1,1 }; // white specular

	Vec3 color = (ambient + diffuse + specular)*light->getIntensity(pos);
	return color.clamp(0.0f, 1.0f);
}
