#pragma once

#include <memory>

#include "Buffer.h"
#include "Vertex.h"

class Camera;
struct Intersection;
class Light;
class Line;
class Material;
struct Ray;
class Scene;
struct Vertex;

class Renderer {
private:
	static constexpr int MAX_DEPTH = 4; // Maximum recursion depth for ray tracing
	const int SAMPLES_PER_LIGHT = 4;
	const int SAMPLES_PER_PIXEL = 8;

	bool firstFrameSaved = false;
public:
	int screenWidth, screenHeight;
	Buffer<uint32_t> framebuffer;
	Buffer<float> zbuffer;
	
	// G-Buffer for SSAO/SSGI
	Buffer<glm::vec3> gBufferPosition;  // World position
	Buffer<glm::vec3> gBufferNormal;    // World normal
	Buffer<glm::vec3> gBufferAlbedo;    // Base color
	Buffer<uint32_t> gBufferColor;      // Direct lighting result
	
	// SSAO/SSGI settings
	static constexpr int SSAO_SAMPLES = 16;
	static constexpr float SSAO_RADIUS = 0.2f;
	static constexpr float SSAO_BIAS = 0.005f;
	static constexpr int SSGI_SAMPLES = 16;
	static constexpr float SSGI_RADIUS = 1.0f;
	std::vector<glm::vec3> ssaoKernel;
	std::vector<glm::vec3> ssaoNoise;
	
	// Lighting intensity controls
	float directLightIntensity = 1.0f;
	float ssaoIntensity = 4.f;
	float ssgiIntensity = 2.f;
	float ambientIntensity = 0.1f;
	
	// Shadow mapping
	static constexpr int SHADOW_MAP_SIZE = 256;
	Buffer<float> shadowMap;
	glm::mat4 lightViewMatrix;
	glm::mat4 lightProjectionMatrix;
	glm::mat4 lightViewProjectionMatrix;
	bool lightMatricesValid = false;
	glm::vec3 lastLightPosition = glm::vec3(0.0f);
private:
	// rasterization
	glm::vec3 sampleTexture(const std::vector<uint32_t>& textureData, glm::vec2 uv, int texWidth, int texHeight);
    bool _isBackFacingViewSpace( const glm::vec3& w0, const glm::vec3& w1, const glm::vec3& w2, const glm::vec3& cameraPosition);
    static bool _insideTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	static std::vector<glm::vec3> clipToScreen(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, int screenWidth, int screenHeight);
	glm::vec3 ndcToScreen(const glm::vec3& ndc) const;
	glm::vec3 ndcToShadowMapScreen(const glm::vec3& ndc) const;
    void clip_triangle_against_near_plane(const VertexShaderOutput& v0, const VertexShaderOutput& v1, const VertexShaderOutput& v2,
        std::vector<std::array<VertexShaderOutput, 3>>& clipped_tris);
	glm::vec3 _computePhongColor(const glm::vec3& pos, const glm::vec3& normal, const std::shared_ptr<Light>& light, const glm::vec3& cameraPos, const glm::vec3& baseColor);

	void _drawTrianglePhong(
		const VertexShaderOutput& v0, const VertexShaderOutput& v1, const VertexShaderOutput& v2,
        const glm::vec3& s0, const glm::vec3& s1, const glm::vec3& s2,
		const std::vector<std::shared_ptr< Light >>& lights, const Camera& camera, std::shared_ptr<Material> material);

	// Shadow mapping
	void renderShadowMap(const Scene& scene, const std::shared_ptr<Light>& light);
	void _drawTriangleDepthOnly(
		const VertexShaderOutput& v0, const VertexShaderOutput& v1, const VertexShaderOutput& v2,
		const glm::vec3& s0, const glm::vec3& s1, const glm::vec3& s2);
	float sampleShadowMap(const glm::vec3& worldPos) const;
	void setupLightMatrices(const std::shared_ptr<Light>& light);

	// SSAO/SSGI
	void generateSSAOKernel();
	void generateSSAONoise();
	void renderGBuffer(Scene scene);
	void _drawTriangleGBuffer(
		const VertexShaderOutput& v0, const VertexShaderOutput& v1, const VertexShaderOutput& v2,
		const glm::vec3& s0, const glm::vec3& s1, const glm::vec3& s2,
		std::shared_ptr<Material> material);
	float computeSSAO(int x, int y, Camera& camera);
	glm::vec3 computeSSGI(int x, int y, Camera& camera);
	glm::vec3 getRandomVector(int x, int y);
	glm::vec3 screenToWorldPosition(float x, float y, float depth, const glm::mat4& invViewProjMatrix);
	
	// ray tracing
	Ray computeReflectedRay(const Ray& ray, const Intersection& isect);
	Ray computeRefractedRay(const Ray& ray, const Intersection& isect);

	float fresnelSchlick(float cosTheta, float ior);
	bool isInShadow(const glm::vec3& point, const Scene& scene, const glm::vec3& lightPos);
	float computeSoftShadow(const glm::vec3& point, const Scene& scene, const glm::vec3& lightPos, int numSamples);
	glm::vec3 traceRay(const Ray& ray, const Scene& scene, int depth);

public:
	void clearBuffers();
    const Buffer<uint32_t>& getBuffer() const { return framebuffer; }
	void render(Scene scene);
	void renderWithSSAO(Scene scene);  // New method with SSAO/SSGI
	void renderRayTracing(Scene scene);

	Renderer(int width, int height);

};