#pragma once

#include <memory>

#include "Buffer.h"

class Camera;
class Light;
class Scene;
struct Vertex;

class Renderer {
public:
	int screenWidth, screenHeight;
	Buffer<uint32_t> framebuffer;
	Buffer<float> zbuffer;
private:
	Vec3 _ProjectToScreen(const Vec3& v, const Mat4& mvp, int w, int h);
	static bool _isBackFacing(const Vec3& v0, const Vec3& v1, const Vec3& v2);
	Vec3 _computePhongColor(const Vec3& pos, const Vec3& normal, const std::shared_ptr<Light>& light, const Vec3& cameraPos, const Vec3& baseColor);
	void _drawTriangleFilled(Vec3 v0, Vec3 v1, Vec3 v2, uint32_t color, int w, int h);

	void _drawTrianglePhong(
		const Vertex& v0, const Vertex& v1, const Vertex& v2,
		const Vec3& w0, const Vec3& w1, const Vec3& w2,
		const Vec3& s0, const Vec3& s1, const Vec3& s2,
		const std::vector<std::shared_ptr< Light >>& light, const Camera& camera,
		const Vec3& baseColor);

public:
	void clearBuffers();
	void render(Scene scene);

	Renderer(int width, int height) : screenWidth(width), screenHeight(height) {
		framebuffer = Buffer<uint32_t>(width, height);
		zbuffer = Buffer<float>(width, height);
		clearBuffers();
	}

};