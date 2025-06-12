#pragma once

#include <memory>

#include "Buffer.h"
#include "Vertex.h"

class Camera;
class Light;
class Scene;
struct Vertex;

class Renderer {
private:
	struct ClipVertex {
		Vertex vertex;
		glm::vec4 clipPos;
		float invW;
	};
	bool inside(const glm::vec4& v, int plane);
	ClipVertex intersect(const ClipVertex& v1, const ClipVertex& v2, int plane);
	float computeT(const glm::vec4& v1, const glm::vec4& v2, int plane);
	std::vector<ClipVertex> clipTriangle(const ClipVertex& v0, const ClipVertex& v1, const ClipVertex& v2);
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

	static std::vector<glm::vec3> clipToScreen(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, int screenWidth, int screenHeight);
	static glm::vec4 _screenToClip(const glm::vec4& s, int screenWidth, int screenHeight);
	glm::vec4 _ProjectToScreen(const glm::vec3& v, const glm::mat4& mvp, int w, int h);
	glm::vec3 _computePhongColor(const glm::vec3& pos, const glm::vec3& normal, const std::shared_ptr<Light>& light, const glm::vec3& cameraPos, const glm::vec3& baseColor);

	void _drawTrianglePhong(
		const Vertex& v0_, const Vertex& v1_, const Vertex& v2_,
		const glm::vec3& s0_, const glm::vec3& s1_, const glm::vec3& s2_,
		const float& w0_, const float& w1_, const float& w2_,
		const std::vector<std::shared_ptr< Light >>& lights, const Camera& camera);

public:
	void clearBuffers();
	void render(Scene scene);

	Renderer(int width, int height);

};