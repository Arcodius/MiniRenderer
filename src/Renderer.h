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
	int textureWidth = 0, textureHeight = 0; // texture size
	std::vector<uint32_t> textureData;
private:
	glm::vec3 sampleTexture(const std::vector<uint32_t>& textureData, glm::vec2 uv, int texWidth, int texHeight);
    static bool _isBackFacing(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);
    static bool _insideTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
	glm::vec3 _ProjectToScreen(const glm::vec3& v, const glm::mat4& mvp, int w, int h);
	glm::vec3 _computePhongColor(const glm::vec3& pos, const glm::vec3& normal, const std::shared_ptr<Light>& light, const glm::vec3& cameraPos, const glm::vec3& baseColor);
	void _drawTriangleFilled(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, uint32_t color, int w, int h);

	void _drawTrianglePhong(
		const glm::vec3& w0, const glm::vec3& w1, const glm::vec3& w2,
		const glm::vec3& s0, const glm::vec3& s1, const glm::vec3& s2,
		glm::vec3 n0, glm::vec3 n1, glm::vec3 n2,
		glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2,
		const std::vector<std::shared_ptr< Light >>& light, const Camera& camera);

public:
	void clearBuffers();
	void render(Scene scene);

	Renderer(int width, int height);

};