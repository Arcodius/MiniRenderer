#include "Renderer.h"
//#include "Debug.h"

Vec3 ProjectToScreen(const Vec3& v, const Mat4& viewProj, int w, int h) {
	Vec4 p = viewProj * Vec4(v, 1.0f);
	if (p.w != 0.0f) p /= p.w;

	return Vec3{
		(p.x * 0.5f + 0.5f) * w,
		(1.0f - (p.y * 0.5f + 0.5f)) * h, // y 反向映射到窗口
		p.z
	};
}

void Renderer::render(HDC hdc, Scene scene) {
	Mat4 viewMatrix = scene.camera.getViewMatrix();
	Mat4 projectionMatrix = scene.camera.getProjectionMatrix();
	Mat4 vp = projectionMatrix * viewMatrix;

	for (Object& object : scene.objects) {
		Mat4 modelMatrix = object.getMatrix();
		Mat4 mvp = vp * modelMatrix;

		const Mesh& mesh = object.getMesh();

		const std::vector<Vertex>& vertices = mesh.vertices;
		const std::vector<unsigned int>& indices = mesh.indices;

		for (size_t i = 0; i < indices.size(); i += 3) {
			std::cout << "v0: " << vertices[indices[i]].position << std::endl;
			std::cout << "v1: " << vertices[indices[i + 1]].position << std::endl;
			std::cout << "v2: " << vertices[indices[i + 2]].position << std::endl << std::endl;
			Vec3 v0 = ProjectToScreen(vertices[indices[i]].position, mvp, screenWidth, screenHeight);
			Vec3 v1 = ProjectToScreen(vertices[indices[i + 1]].position, mvp, screenWidth, screenHeight);
			Vec3 v2 = ProjectToScreen(vertices[indices[i + 2]].position, mvp, screenWidth, screenHeight);
            std::cout<< "v0: " << v0.x << ", " << v0.y << ", " << v0.z << std::endl;
			std::cout<< "v1: " << v1.x << ", " << v1.y << ", " << v1.z << std::endl;
			std::cout<< "v2: " << v2.x << ", " << v2.y << ", " << v2.z << std::endl << std::endl;

			MoveToEx(hdc, static_cast<int>(v0.x), static_cast<int>(v0.y), nullptr);
			LineTo(hdc, static_cast<int>(v1.x), static_cast<int>(v1.y));
			LineTo(hdc, static_cast<int>(v2.x), static_cast<int>(v2.y));
			LineTo(hdc, static_cast<int>(v0.x), static_cast<int>(v0.y));
		}
	}
}