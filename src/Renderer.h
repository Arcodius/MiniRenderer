#pragma once

#include <memory>

#include "Buffer.h"
#include "Vertex.h"

// Forward declarations
class Camera;
struct Intersection;
class Light;
class Line;
class Material;
struct Ray;
class Scene;
struct Vertex;
class CudaRenderer;

class Renderer {
private:
	static constexpr int MAX_DEPTH = 8; // Maximum recursion depth for ray tracing
	const int SAMPLES_PER_LIGHT = 4;
	const int SAMPLES_PER_PIXEL = 8;

	bool firstFrameSaved = false;
	
	// CUDA acceleration
	bool enableCuda = false;
	void* cudaRenderer; // Use void* to avoid incomplete type issues
public:
	int screenWidth, screenHeight;
	Buffer<uint32_t> framebuffer;
	Buffer<float> zbuffer;
private:
	// rasterization
	glm::vec3 sampleTexture(const std::vector<uint32_t>& textureData, glm::vec2 uv, int texWidth, int texHeight);
    bool _isBackFacingViewSpace( const glm::vec3& w0, const glm::vec3& w1, const glm::vec3& w2, const glm::vec3& cameraPosition);
    static bool _insideTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	static std::vector<glm::vec3> clipToScreen(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, int screenWidth, int screenHeight);
	glm::vec3 ndcToScreen(const glm::vec3& ndc) const;
    void clip_triangle_against_near_plane(const VertexShaderOutput& v0, const VertexShaderOutput& v1, const VertexShaderOutput& v2,
        std::vector<std::array<VertexShaderOutput, 3>>& clipped_tris);
	glm::vec3 _computePhongColor(const glm::vec3& pos, const glm::vec3& normal, const std::shared_ptr<Light>& light, const glm::vec3& cameraPos, const glm::vec3& baseColor);

	void _drawTrianglePhong(
		const VertexShaderOutput& v0, const VertexShaderOutput& v1, const VertexShaderOutput& v2,
        const glm::vec3& s0, const glm::vec3& s1, const glm::vec3& s2,
		const std::vector<std::shared_ptr< Light >>& lights, const Camera& camera, std::shared_ptr<Material> material);

	// ray tracing
	Ray computeReflectedRay(const Ray& ray, const Intersection& isect);
	Ray computeRefractedRay(const Ray& ray, const Intersection& isect);

	float fresnelSchlick(float cosTheta, float ior);
	bool isInShadow(const glm::vec3& point, const Scene& scene, const glm::vec3& lightPos);
	float computeSoftShadow(const glm::vec3& point, const Scene& scene, const glm::vec3& lightPos, int numSamples);
	glm::vec3 traceRay(const Ray& ray, const Scene& scene, int depth);
	
	// CUDA helper methods
	void renderWithCuda(Scene& scene, bool raytracing);

public:
	void clearBuffers();
    const Buffer<uint32_t>& getBuffer() const { return framebuffer; }
	void render(Scene scene);
	void renderRayTracing(Scene scene);
	
	// CUDA methods
	void enableCudaAcceleration(bool enable = true);
	bool isCudaEnabled() const { return enableCuda; }

	Renderer(int width, int height);
	~Renderer(); // Add destructor declaration

};