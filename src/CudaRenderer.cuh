#pragma once

#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <curand_kernel.h>
#include "MyMath.h"
#include "Vertex.h"
#include "Ray.h"
#include "Intersection.h"

// CUDA error checking macro
#define CUDA_CHECK(call) \
    do { \
        cudaError_t error = call; \
        if (error != cudaSuccess) { \
            fprintf(stderr, "CUDA error at %s:%d - %s\n", __FILE__, __LINE__, cudaGetErrorString(error)); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

// Device structures for CUDA
struct CudaTriangle {
    glm::vec3 v0, v1, v2;
    glm::vec3 n0, n1, n2;
    glm::vec2 uv0, uv1, uv2;
    int materialId;
};

struct CudaMaterial {
    glm::vec3 baseColor;
    float metallic;
    float roughness;
    float transparency;
    float ior;
};

struct CudaLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    int type; // 0: point light, 1: area light
};

struct CudaCamera {
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 up;
    glm::vec3 right;
    float fov;
    float aspect;
    float nearPlane;
    float farPlane;
};

// CUDA kernel declarations
extern "C" {
    void cuda_render_rasterize(
        uint32_t* framebuffer,
        float* zbuffer,
        int width, int height,
        const VertexShaderOutput* vertices,
        const int* indices,
        int triangleCount,
        const CudaLight* lights,
        int lightCount,
        const CudaMaterial* materials,
        const CudaCamera& camera
    );

    void cuda_render_raytrace(
        uint32_t* framebuffer,
        int width, int height,
        const CudaTriangle* triangles,
        int triangleCount,
        const CudaMaterial* materials,
        const CudaLight* lights,
        int lightCount,
        const CudaCamera& camera,
        int samplesPerPixel,
        int maxDepth
    );

    void cuda_clear_buffers(uint32_t* framebuffer, float* zbuffer, int size);
}

// Host-side CUDA wrapper class
class CudaRenderer {
private:
    // Device pointers
    uint32_t* d_framebuffer;
    float* d_zbuffer;
    CudaTriangle* d_triangles;
    CudaMaterial* d_materials;
    CudaLight* d_lights;
    VertexShaderOutput* d_vertices;
    int* d_indices;

    // Buffer sizes
    int framebuffer_size;
    int max_triangles;
    int max_materials;
    int max_lights;
    int max_vertices;
    int max_indices;

    bool initialized;

public:
    CudaRenderer();
    ~CudaRenderer();

    void initialize(int width, int height, int maxTriangles = 100000, 
                   int maxMaterials = 100, int maxLights = 32);
    void cleanup();

    void renderRasterization(
        uint32_t* framebuffer,
        float* zbuffer,
        int width, int height,
        const VertexShaderOutput* vertices,
        const int* indices,
        int triangleCount,
        const CudaLight* lights,
        int lightCount,
        const CudaMaterial* materials,
        const CudaCamera& camera
    );

    void renderRaytracing(
        uint32_t* framebuffer,
        int width, int height,
        const CudaTriangle* triangles,
        int triangleCount,
        const CudaMaterial* materials,
        int materialCount,
        const CudaLight* lights,
        int lightCount,
        const CudaCamera& camera,
        int samplesPerPixel = 8,
        int maxDepth = 4
    );

    void clearBuffers(uint32_t* framebuffer, float* zbuffer, int size);

    // Test function for CUDA functionality
    void renderTestPattern(uint32_t* framebuffer, int width, int height);
};
