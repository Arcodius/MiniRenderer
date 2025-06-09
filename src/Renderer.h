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
    static bool _isBackFacing(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);
    static bool _insideTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
	glm::vec3 _ProjectToScreen(const glm::vec3& v, const glm::mat4& mvp, int w, int h);
	glm::vec3 _computePhongColor(const glm::vec3& pos, const glm::vec3& normal, const std::shared_ptr<Light>& light, const glm::vec3& cameraPos, const glm::vec3& baseColor);
	void _drawTriangleFilled(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, uint32_t color, int w, int h);

	void _drawTrianglePhong(
		const Vertex& v0, const Vertex& v1, const Vertex& v2,
		const glm::vec3& w0, const glm::vec3& w1, const glm::vec3& w2,
		const glm::vec3& s0, const glm::vec3& s1, const glm::vec3& s2,
		const std::vector<std::shared_ptr< Light >>& light, const Camera& camera,
		const glm::vec3& baseColor);

public:
	void clearBuffers();
	void render(Scene scene);

	Renderer(int width, int height) : screenWidth(width), screenHeight(height) {
		framebuffer = Buffer<uint32_t>(width, height);
		zbuffer = Buffer<float>(width, height);
		clearBuffers();
	}

};