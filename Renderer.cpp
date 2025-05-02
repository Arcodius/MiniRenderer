#include "Renderer.h"

#include "Scene.h"
//#include "Debug.h"
//#define DEBUG_MODE

void Renderer::_drawFilledTriangle(Vec3 v0, Vec3 v1, Vec3 v2, uint32_t color, int w, int h) {
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
			if (z < depthbuffer[idx]) {
				depthbuffer[idx] = z;
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

void Renderer::clearBuffers() {
	framebuffer.clear(0xFF000000); // Clear to black
	depthbuffer.clear(1.0f); // Clear depth buffer to max depth
}

// viewport transform: from NDC to window
Vec3 ProjectToScreen(const Vec3& v, const Mat4& mvp, int w, int h) {
	// local -> world -> view -> clip
	Vec4 p = mvp * Vec4(v, 1.0f);
	//// view clipping
	//if (p.x < -p.w || p.x > p.w || p.y < -p.w || p.y > p.w || p.z < -p.w || p.z > p.w)
	//{
	//	return {}; // drop the vertex
	//}
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

		for (size_t i = 0; i < indices.size(); i += 3) {
			#ifdef DEBUG_MODE
			std::cout << "v0: " << vertices[indices[i]].position << std::endl;
			std::cout << "v1: " << vertices[indices[i + 1]].position << std::endl;
			std::cout << "v2: " << vertices[indices[i + 2]].position << std::endl << std::endl;
			#endif
			Vec3 v0 = ProjectToScreen(vertices[indices[i]].position, mvp, screenWidth, screenHeight);
			Vec3 v1 = ProjectToScreen(vertices[indices[i + 1]].position, mvp, screenWidth, screenHeight);
			Vec3 v2 = ProjectToScreen(vertices[indices[i + 2]].position, mvp, screenWidth, screenHeight);

			if (_isBackFacing(v0, v1, v2)) continue;
			_drawFilledTriangle(v0, v1, v2, 0xFFFFFF, screenWidth, screenHeight);

#ifdef DEBUG_MODE
			std::cout << "v0: " << v0.x << ", " << v0.y << ", " << v0.z << std::endl;
			std::cout << "v1: " << v1.x << ", " << v1.y << ", " << v1.z << std::endl;
			std::cout << "v2: " << v2.x << ", " << v2.y << ", " << v2.z << std::endl << std::endl;
			MoveToEx(hdc, static_cast<int>(v0.x), static_cast<int>(v0.y), nullptr);
			LineTo(hdc, static_cast<int>(v1.x), static_cast<int>(v1.y));
			LineTo(hdc, static_cast<int>(v2.x), static_cast<int>(v2.y));
			LineTo(hdc, static_cast<int>(v0.x), static_cast<int>(v0.y));
#endif
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

Vec3 computePhongColor(
	const Vec3& pos,
	const Vec3& normal,
	const PointLight& light,
	const Vec3& cameraPos,
	const Vec3& baseColor)
{
	Vec3 L = (light.position - pos).normalized();
	Vec3 N = normal.normalized();
	Vec3 V = (cameraPos - pos).normalized();
	Vec3 R = (2.0f * N.dot(L) * N - L).normalized();

	float diff = max(0.0f, N.dot(L));
	float spec = std::pow(max(0.0f, R.dot(V)), 16.0f); // shininess

	Vec3 ambient = 0.1f * baseColor;
	Vec3 diffuse = diff * baseColor;
	Vec3 specular = spec * Vec3{ 1,1,1 }; // white specular

	Vec3 color = ambient + diffuse + specular;
	return color.clamp(0.0f, 1.0f);
}
