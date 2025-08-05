#include "CudaRenderer.cuh"
#include "Color.h"
#include <stdio.h>
#include <float.h>

// GLM for device code
#define GLM_FORCE_CUDA
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Device constants
__constant__ float EPSILON_CUDA = 1e-6f;
__constant__ int MAX_DEPTH_CUDA = 4;

// Device utility functions
__device__ bool insideTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    return (glm::cross(b - a, p - a).z >= 0.0f &&
            glm::cross(c - b, p - b).z >= 0.0f &&
            glm::cross(a - c, p - c).z >= 0.0f);
}

__device__ glm::vec3 barycentric(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    float area = glm::cross(b - a, c - a).z;
    if (fabs(area) < EPSILON_CUDA) return glm::vec3(0.0f);
    
    float alpha = glm::cross(b - p, c - p).z / area;
    float beta = glm::cross(c - p, a - p).z / area;
    float gamma = glm::cross(a - p, b - p).z / area;
    
    return glm::vec3(alpha, beta, gamma);
}

// 一个健壮的、可在 CUDA 设备上运行的颜色转换函数
__device__ uint32_t vec3ToColor(const glm::vec3& color) {
    // 步骤 1: Gamma 校正 (你在内核末尾已经做了，这里假设传入的已经是校正后的颜色)
    // 如果你没有在内核里做，可以在这里做

    // 步骤 2: 将颜色分量限制在 [0.0, 1.0] 的范围内，防止异常值
    float r = fmaxf(0.0f, fminf(1.0f, color.r));
    float g = fmaxf(0.0f, fminf(1.0f, color.g));
    float b = fmaxf(0.0f, fminf(1.0f, color.b));

    // 步骤 3: 将 [0.0, 1.0] 的浮点数转换为 [0, 255] 的整数
    // 乘以 255.999 是一个常用技巧，可以确保 1.0f 被正确地映射到 255
    uint8_t ri = static_cast<uint8_t>(r * 255.999f);
    uint8_t gi = static_cast<uint8_t>(g * 255.999f);
    uint8_t bi = static_cast<uint8_t>(b * 255.999f);

    // 步骤 4: 将 R, G, B 分量打包成一个 32 位整数 (uint32_t)
    // 常见的格式是 0xAARRGGBB (Alpha, Red, Green, Blue)。
    // 这里我们将 Alpha 通道设置为 255 (完全不透明)。
    return (255 << 24) | (ri << 16) | (gi << 8) | bi;
}

// 设备函数：Schlick近似法计算菲涅尔反射率
__device__ float fresnelSchlick(float cosTheta, float ior) {
    float r0 = (1.0f - ior) / (1.0f + ior);
    r0 = r0 * r0;
    // 钳制cosTheta以避免浮点数问题
    cosTheta = fmaxf(0.0f, fminf(1.0f, cosTheta));
    return r0 + (1.0f - r0) * powf(1.0f - cosTheta, 5.0f);
}

// 设备函数：计算折射方向，并处理全内反射
__device__ glm::vec3 refract_dir(const glm::vec3& I, const glm::vec3& N, float iorRatio) {
    float cosI = -glm::dot(N, I);
    float sinT2 = iorRatio * iorRatio * (1.0f - cosI * cosI);
    if (sinT2 > 1.0f) return glm::vec3(0.0f); // 全内反射发生，返回零向量
    float cosT = sqrtf(1.0f - sinT2);
    return iorRatio * I + (iorRatio * cosI - cosT) * N;
}

// Simple test kernel for CUDA functionality
__global__ void cuda_test_kernel(uint32_t* framebuffer, int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x >= width || y >= height) return;
    
    // Create a simple, guaranteed visible pattern
    uint32_t color;
    
    if ((x / 20 + y / 20) % 2 == 0) {
        // Red squares
        color = 0xFFFF0000;
    } else {
        // Blue squares  
        color = 0xFF0000FF;
    }
    
    framebuffer[y * width + x] = color;
}

// Rasterization kernel
__global__ void cuda_rasterize_kernel(
    uint32_t* framebuffer,
    float* zbuffer,
    int width, int height,
    const VertexShaderOutput* vertices,
    const int* indices,
    int triangleCount,
    const CudaLight* lights,
    int lightCount,
    const CudaMaterial* materials,
    const CudaCamera camera
) {
    int triangleId = blockIdx.x * blockDim.x + threadIdx.x;
    if (triangleId >= triangleCount) return;

    // Get triangle vertices
    int i0 = indices[triangleId * 3];
    int i1 = indices[triangleId * 3 + 1];
    int i2 = indices[triangleId * 3 + 2];
    
    VertexShaderOutput v0 = vertices[i0];
    VertexShaderOutput v1 = vertices[i1];
    VertexShaderOutput v2 = vertices[i2];

    // NDC to screen space
    glm::vec3 s0((v0.clipPos.x / v0.clipPos.w + 1.0f) * 0.5f * width,
                 (1.0f - v0.clipPos.y / v0.clipPos.w) * 0.5f * height,
                 (v0.clipPos.z / v0.clipPos.w + 1.0f) * 0.5f);
    glm::vec3 s1((v1.clipPos.x / v1.clipPos.w + 1.0f) * 0.5f * width,
                 (1.0f - v1.clipPos.y / v1.clipPos.w) * 0.5f * height,
                 (v1.clipPos.z / v1.clipPos.w + 1.0f) * 0.5f);
    glm::vec3 s2((v2.clipPos.x / v2.clipPos.w + 1.0f) * 0.5f * width,
                 (1.0f - v2.clipPos.y / v2.clipPos.w) * 0.5f * height,
                 (v2.clipPos.z / v2.clipPos.w + 1.0f) * 0.5f);

    // Triangle bounding box
    int minX = max(0, (int)floor(min(min(s0.x, s1.x), s2.x)));
    int maxX = min(width - 1, (int)ceil(max(max(s0.x, s1.x), s2.x)));
    int minY = max(0, (int)floor(min(min(s0.y, s1.y), s2.y)));
    int maxY = min(height - 1, (int)ceil(max(max(s0.y, s1.y), s2.y)));

    float area = glm::cross(s1 - s0, s2 - s0).z;
    if (fabs(area) < EPSILON_CUDA) return;

    // Perspective correction
    float invW0 = 1.0f / v0.w;
    float invW1 = 1.0f / v1.w;
    float invW2 = 1.0f / v2.w;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            glm::vec3 p(x + 0.5f, y + 0.5f, 0.0f);
            if (insideTriangle(p, s0, s1, s2)) {
                glm::vec3 bary = barycentric(p, s0, s1, s2);
                float z = bary.x * s0.z + bary.y * s1.z + bary.z * s2.z;
                
                int idx = y * width + x;
                if (z < zbuffer[idx]) {
                    // Atomic compare and swap for depth testing
                    float old = atomicExch(&zbuffer[idx], z);
                    if (z < old) {
                        // Interpolate attributes
                        float invW = bary.x * invW0 + bary.y * invW1 + bary.z * invW2;
                        glm::vec3 worldPos = (v0.worldPos * bary.x * invW0 + 
                                            v1.worldPos * bary.y * invW1 + 
                                            v2.worldPos * bary.z * invW2) / invW;
                        glm::vec3 normal = glm::normalize(v0.normal * bary.x * invW0 + 
                                                        v1.normal * bary.y * invW1 + 
                                                        v2.normal * bary.z * invW2);
                        glm::vec2 uv = (v0.uv * bary.x * invW0 + 
                                       v1.uv * bary.y * invW1 + 
                                       v2.uv * bary.z * invW2) / invW;

                        // Simple Phong shading
                        glm::vec3 color(0.1f); // ambient
                        if (lightCount > 0) {
                            glm::vec3 viewDir = glm::normalize(camera.position - worldPos);
                            for (int i = 0; i < lightCount; ++i) {
                                glm::vec3 lightDir = glm::normalize(lights[i].position - worldPos);
                                float diff = fmaxf(0.0f, glm::dot(normal, lightDir));
                                glm::vec3 reflectDir = glm::normalize(2.0f * glm::dot(normal, lightDir) * normal - lightDir);
                                float spec = powf(fmaxf(0.0f, glm::dot(reflectDir, viewDir)), 16.0f);
                                
                                color += (diff + spec) * lights[i].color * lights[i].intensity;
                            }
                        }
                        
                        framebuffer[idx] = vec3ToColor(color);
                    } else {
                        zbuffer[idx] = old; // restore original depth
                    }
                }
            }
        }
    }
}

// Ray tracing kernels
__device__ bool intersectTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                const CudaTriangle& tri, float& t, glm::vec2& uv) {
    glm::vec3 edge1 = tri.v1 - tri.v0;
    glm::vec3 edge2 = tri.v2 - tri.v0;
    glm::vec3 h = glm::cross(rayDir, edge2);
    float a = glm::dot(edge1, h);
    
    if (a > -EPSILON_CUDA && a < EPSILON_CUDA) return false;
    
    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - tri.v0;
    float u = f * glm::dot(s, h);
    
    if (u < 0.0f || u > 1.0f) return false;
    
    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayDir, q);
    
    if (v < 0.0f || u + v > 1.0f) return false;
    
    t = f * glm::dot(edge2, q);
    uv = glm::vec2(u, v);
    
    return t > EPSILON_CUDA;
}

// Create orthonormal basis from normal vector
// This function is used to create a tangent and bitangent for normal mapping
__device__ void createOrthonormalBasis(const glm::vec3& n, glm::vec3& t, glm::vec3& b) {
    if (fabsf(n.x) > fabsf(n.z)) {
        t = glm::vec3(-n.y, n.x, 0.0f);
    } else {
        t = glm::vec3(0.0f, -n.z, n.y);
    }
    t = glm::normalize(t);
    b = glm::cross(n, t);
}

__global__ void cuda_raytrace_kernel(
    uint32_t* framebuffer,
    int width, int height,
    const CudaTriangle* triangles,
    int triangleCount,
    const CudaMaterial* materials,
    const CudaLight* lights,
    int lightCount,
    const CudaCamera camera,
    int samplesPerPixel,
    int maxDepth
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) return;

    // 初始化随机状态
    curandState state;
    curand_init((unsigned long long)y * width + x, 0, 0, &state);

    glm::vec3 color(0.0f);

    if (triangleCount == 0) {
        float u = (float)x / width;
        float v = (float)y / height;
        color = glm::vec3(u, v, 0.5f);
        framebuffer[y * width + x] = vec3ToColor(color);
        return;
    }

    // 每个像素多次采样以实现抗锯齿
    for (int s = 0; s < samplesPerPixel; ++s) {
        float px = (float)x + curand_uniform(&state);
        float py = (float)y + curand_uniform(&state);

        // 生成光线
        float u = (px / width) * 2.0f - 1.0f;
        float v = (py / height) * 2.0f - 1.0f;
        v = -v; // 翻转y轴

        float fovRad = camera.fov * 3.14159265359f / 180.0f;
        float scale = tanf(fovRad * 0.5f);
        u *= scale * camera.aspect;
        v *= scale;

        glm::vec3 rayDir = glm::normalize(camera.forward + u * camera.right + v * camera.up);
        
        if (isnan(rayDir.x) || isnan(rayDir.y) || isnan(rayDir.z)) {
            color += glm::vec3(1.0f, 0.0f, 0.0f);
            continue;
        }

        // 追踪光线
        glm::vec3 rayOrigin = camera.position;
        glm::vec3 sampleColor(0.0f);
        glm::vec3 throughput(1.0f);

        for (int depth = 0; depth < maxDepth; ++depth) {
            float closestT = FLT_MAX;
            int hitTriangle = -1;
            glm::vec2 hitUV;

            // 寻找最近的交点
            for (int i = 0; i < triangleCount; ++i) {
                float t;
                glm::vec2 uv;
                if (intersectTriangle(rayOrigin, rayDir, triangles[i], t, uv)) {
                    if (t < closestT) {
                        closestT = t;
                        hitTriangle = i;
                        hitUV = uv;
                    }
                }
            }

            if (hitTriangle == -1) {
                sampleColor += throughput * glm::vec3(0.3f, 0.5f, 0.8f);
                break;
            }

            glm::vec3 hitPoint = rayOrigin + closestT * rayDir;
            const CudaTriangle& tri = triangles[hitTriangle];
            const CudaMaterial& mat = materials[tri.materialId];

            float w = 1.0f - hitUV.x - hitUV.y;
            glm::vec3 shadingNormal = w * tri.n0 + hitUV.x * tri.n1 + hitUV.y * tri.n2;

            if (glm::dot(shadingNormal, shadingNormal) < 1e-6f) {
                shadingNormal = glm::cross(tri.v1 - tri.v0, tri.v2 - tri.v0);
                if (glm::dot(shadingNormal, shadingNormal) < 1e-6f) {
                    sampleColor += throughput * glm::vec3(1.0f, 0.0f, 1.0f);
                    break;
                }
            }
            shadingNormal = glm::normalize(shadingNormal);
            // glm::vec3 frontNormal = glm::faceforward(shadingNormal, -rayDir, shadingNormal);
            glm::vec3 frontNormal = shadingNormal; // 假设法线总是朝向外部

            // --- 【核心修改】统一的、基于物理的材质模型 ---

            // 1. 为不透明物体添加直接光照贡献 (Next Event Estimation)
            if (mat.transparency < 0.5f) {
                glm::vec3 lightContrib(0.0f); 
                for (int i = 0; i < lightCount; ++i) {
                    glm::vec3 toLight = lights[i].position - hitPoint;
                    float lightDistSq = glm::dot(toLight, toLight);
                    if (lightDistSq < 1e-6f) continue;
                    float lightDist = sqrtf(lightDistSq);
                    glm::vec3 lightDir = toLight / lightDist;

                    // --- 【核心修正】阴影光线检测 ---
                    bool inShadow = false;
                    glm::vec3 shadowRayOrigin = hitPoint + frontNormal * 1e-4f; // 加上一个小的偏移量防止自相交
                    for (int j = 0; j < triangleCount; ++j) {
                        float t_shadow;
                        glm::vec2 uv_shadow;
                        if (intersectTriangle(shadowRayOrigin, lightDir, triangles[j], t_shadow, uv_shadow)) {
                            // 如果交点在物体表面和光源之间，则说明该点处于阴影中
                            if (t_shadow > 0.0f && t_shadow < lightDist) {
                                inShadow = true;
                                break; // 找到遮挡物，无需继续检查
                            }
                        }
                    }

                    // 如果不在阴影中，才添加该光源的贡献
                    if (!inShadow) {
                        float NdotL = fmaxf(0.0f, glm::dot(frontNormal, lightDir));
                        // 这里可以根据距离添加衰减: lightContrib += (NdotL * lights[i].color * lights[i].intensity) / lightDistSq;
                        lightContrib += NdotL * lights[i].color * lights[i].intensity;
                    }
                }
                // 只有漫反射部分接收直接光
                glm::vec3 diffuseColor = (1.0f - mat.metallic) * mat.baseColor;
                sampleColor += throughput * diffuseColor * lightContrib;
            }

            // --- 【优化 1】俄式轮盘赌 (Russian Roulette) ---
            // 在几次弹射后，根据光线能量决定是否继续追踪
            if (depth > 3) {
                float p = fmaxf(throughput.x, fmaxf(throughput.y, throughput.z));
                if (curand_uniform(&state) > p) {
                    break; // 概率性地终止路径
                }
                throughput /= p; // 补偿能量，保持无偏
            }

            // 2. 根据材质类型，决定光线的下一次弹射行为
            
            // --- 情况一: 透明材质 (如玻璃) ---
            if (mat.transparency > 0.5f) {
                glm::vec3 outward_normal;
                float iorRatio;
                float cosTheta = glm::dot(rayDir, frontNormal);
                float reflectance;

                if (cosTheta < 0.0f) { // 光线从外部射入物体
                    outward_normal = frontNormal;
                    iorRatio = 1.0f / mat.ior;
                    cosTheta = -cosTheta;
                } else { // 光线从物体内部射出
                    outward_normal = -frontNormal;
                    iorRatio = mat.ior;
                }

                glm::vec3 refractedDir = refract_dir(rayDir, outward_normal, iorRatio);
                if (glm::dot(refractedDir, refractedDir) < 1e-6f) { // 发生全内反射
                    reflectance = 1.0f;
                } else {
                    reflectance = fresnelSchlick(cosTheta, mat.ior);
                }
                
                if (curand_uniform(&state) < reflectance) { // 概率上选择反射
                    rayDir = glm::reflect(rayDir, frontNormal);
                    rayOrigin = hitPoint + frontNormal * 1e-4f;
                } else { // 概率上选择折射
                    rayDir = glm::normalize(refractedDir);
                    rayOrigin = hitPoint - frontNormal * 1e-4f; // 光线进入物体内部
                    throughput *= mat.baseColor; // 对于有色玻璃，光能被基色过滤
                }
            } 
            // --- 情况二: 不透明材质 ---
            else {
                // 子情况 2a: 电介质 (非金属)
                if (mat.metallic < 0.5f) {
                    float reflectance = fresnelSchlick(-glm::dot(rayDir, frontNormal), mat.ior);
                    if (curand_uniform(&state) < reflectance) { // 镜面反射
                        float roughness = fmaxf(mat.roughness, 0.01f);
                        float alpha = roughness * roughness;
                        float r1 = curand_uniform(&state);
                        float r2 = curand_uniform(&state);
                        float theta = acosf(sqrtf((1.0f - r1) / (1.0f + (alpha * alpha - 1.0f) * r1)));
                        float phi = 2.0f * 3.14159265359f * r2;
                        glm::vec3 h_local(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));
                        glm::vec3 tangent, bitangent;
                        createOrthonormalBasis(frontNormal, tangent, bitangent);
                        glm::vec3 h_world = glm::normalize(h_local.x * tangent + h_local.y * bitangent + h_local.z * frontNormal);
                        rayDir = glm::reflect(rayDir, h_world);
                        rayOrigin = hitPoint + frontNormal * 1e-4f;
                        // 电介质的镜面反射是白色的，所以 throughput 不变
                    } else { // 漫反射
                        glm::vec3 tangent, bitangent;
                        createOrthonormalBasis(frontNormal, tangent, bitangent);
                        float r1 = curand_uniform(&state);
                        float r2 = curand_uniform(&state);
                        float sinTheta = sqrtf(r1);
                        float cosTheta = sqrtf(1.0f - r1);
                        float phi = 2.0f * 3.14159265359f * r2;
                        glm::vec3 localDir(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
                        rayDir = glm::normalize(localDir.x * tangent + localDir.y * bitangent + localDir.z * frontNormal);
                        rayOrigin = hitPoint + frontNormal * 1e-4f;
                        throughput *= mat.baseColor; // 漫反射的光能被基色过滤
                    }
                }
                // 子情况 2b: 金属
                else {
                    // 金属总是镜面反射
                    float roughness = fmaxf(mat.roughness, 0.01f);
                    float alpha = roughness * roughness;
                    float r1 = curand_uniform(&state);
                    float r2 = curand_uniform(&state);
                    float theta = acosf(sqrtf((1.0f - r1) / (1.0f + (alpha * alpha - 1.0f) * r1)));
                    float phi = 2.0f * 3.14159265359f * r2;
                    glm::vec3 h_local(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));
                    glm::vec3 tangent, bitangent;
                    createOrthonormalBasis(frontNormal, tangent, bitangent);
                    glm::vec3 h_world = glm::normalize(h_local.x * tangent + h_local.y * bitangent + h_local.z * frontNormal);
                    rayDir = glm::reflect(rayDir, h_world);
                    rayOrigin = hitPoint + frontNormal * 1e-4f;
                    throughput *= mat.baseColor; // 金属的反射是有颜色的
                }
            }
        }
        // --- 【优化 2】样本辉度钳制 (Sample Clamping) ---
        // 限制单次光路追踪的贡献，有效消除火花/亮斑
        if (!isnan(sampleColor.x) && !isnan(sampleColor.y) && !isnan(sampleColor.z)) {
            sampleColor = glm::min(sampleColor, glm::vec3(4.0f)); // 将单次采样的最大亮度限制在4.0
        } else {
            sampleColor = glm::vec3(0.0f); // 丢弃无效采样
        }
        color += sampleColor;
    }

    color /= float(samplesPerPixel);

    // Gamma 校正
    color.r = powf(fmaxf(color.r, 0.0f), 1.0f / 2.2f);
    color.g = powf(fmaxf(color.g, 0.0f), 1.0f / 2.2f);
    color.b = powf(fmaxf(color.b, 0.0f), 1.0f / 2.2f);

    framebuffer[y * width + x] = vec3ToColor(color);
}

__global__ void cuda_clear_kernel(uint32_t* framebuffer, float* zbuffer, int size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) {
        framebuffer[idx] = 0xFF000000;
        zbuffer[idx] = FLT_MAX;
    }
}

// Host functions
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
    ) {
        dim3 blockSize(256);
        dim3 gridSize((triangleCount + blockSize.x - 1) / blockSize.x);
        
        cuda_rasterize_kernel<<<gridSize, blockSize>>>(
            framebuffer, zbuffer, width, height,
            vertices, indices, triangleCount,
            lights, lightCount, materials, camera
        );
        
        CUDA_CHECK(cudaDeviceSynchronize());
    }

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
    ) {
        dim3 blockSize(16, 16);
        dim3 gridSize((width + blockSize.x - 1) / blockSize.x,
                      (height + blockSize.y - 1) / blockSize.y);
        
        cuda_raytrace_kernel<<<gridSize, blockSize>>>(
            framebuffer, width, height,
            triangles, triangleCount, materials,
            lights, lightCount, camera,
            samplesPerPixel, maxDepth
        );
        
        CUDA_CHECK(cudaDeviceSynchronize());
    }

    void cuda_clear_buffers(uint32_t* framebuffer, float* zbuffer, int size) {
        dim3 blockSize(256);
        dim3 gridSize((size + blockSize.x - 1) / blockSize.x);
        
        cuda_clear_kernel<<<gridSize, blockSize>>>(framebuffer, zbuffer, size);
        
        CUDA_CHECK(cudaDeviceSynchronize());
    }

    void cuda_test_pattern(uint32_t* framebuffer, int width, int height) {
        dim3 blockSize(16, 16);
        dim3 gridSize((width + blockSize.x - 1) / blockSize.x,
                      (height + blockSize.y - 1) / blockSize.y);
        
        cuda_test_kernel<<<gridSize, blockSize>>>(framebuffer, width, height);
        
        CUDA_CHECK(cudaDeviceSynchronize());
    }
}

// CudaRenderer class implementation
CudaRenderer::CudaRenderer() : initialized(false), d_framebuffer(nullptr), d_zbuffer(nullptr),
    d_triangles(nullptr), d_materials(nullptr), d_lights(nullptr),
    d_vertices(nullptr), d_indices(nullptr) {
}

CudaRenderer::~CudaRenderer() {
    cleanup();
}

void CudaRenderer::initialize(int width, int height, int maxTriangles, int maxMaterials, int maxLights) {
    if (initialized) cleanup();
    
    framebuffer_size = width * height;
    max_triangles = maxTriangles;
    max_materials = maxMaterials;
    max_lights = maxLights;
    max_vertices = maxTriangles * 3;
    max_indices = maxTriangles * 3;
    
    // Allocate device memory
    CUDA_CHECK(cudaMalloc(&d_framebuffer, framebuffer_size * sizeof(uint32_t)));
    CUDA_CHECK(cudaMalloc(&d_zbuffer, framebuffer_size * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_triangles, max_triangles * sizeof(CudaTriangle)));
    CUDA_CHECK(cudaMalloc(&d_materials, max_materials * sizeof(CudaMaterial)));
    CUDA_CHECK(cudaMalloc(&d_lights, max_lights * sizeof(CudaLight)));
    CUDA_CHECK(cudaMalloc(&d_vertices, max_vertices * sizeof(VertexShaderOutput)));
    CUDA_CHECK(cudaMalloc(&d_indices, max_indices * sizeof(int)));
    
    initialized = true;
}

void CudaRenderer::cleanup() {
    if (!initialized) return;
    
    cudaFree(d_framebuffer);
    cudaFree(d_zbuffer);
    cudaFree(d_triangles);
    cudaFree(d_materials);
    cudaFree(d_lights);
    cudaFree(d_vertices);
    cudaFree(d_indices);
    
    initialized = false;
}

void CudaRenderer::renderRaytracing(
    uint32_t* framebuffer,
    int width, int height,
    const CudaTriangle* triangles,
    int triangleCount,
    const CudaMaterial* materials,
    int materialCount,
    const CudaLight* lights,
    int lightCount,
    const CudaCamera& camera,
    int samplesPerPixel,
    int maxDepth
) {
    // Copy data to device
    CUDA_CHECK(cudaMemcpy(d_triangles, triangles, triangleCount * sizeof(CudaTriangle), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_materials, materials, materialCount * sizeof(CudaMaterial), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_lights, lights, lightCount * sizeof(CudaLight), cudaMemcpyHostToDevice));
    
    // Call CUDA kernel
    cuda_render_raytrace(
        d_framebuffer, width, height,
        d_triangles, triangleCount, d_materials,
        d_lights, lightCount, camera,
        samplesPerPixel, maxDepth
    );
    
    // Copy result back to host
    CUDA_CHECK(cudaMemcpy(framebuffer, d_framebuffer, width * height * sizeof(uint32_t), cudaMemcpyDeviceToHost));
}

void CudaRenderer::renderRasterization(
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
) {
    // Copy data to device
    CUDA_CHECK(cudaMemcpy(d_vertices, vertices, triangleCount * 3 * sizeof(VertexShaderOutput), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_indices, indices, triangleCount * 3 * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_materials, materials, triangleCount * sizeof(CudaMaterial), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_lights, lights, lightCount * sizeof(CudaLight), cudaMemcpyHostToDevice));
    
    // Call CUDA kernel
    cuda_render_rasterize(
        d_framebuffer, d_zbuffer, width, height,
        d_vertices, d_indices, triangleCount,
        d_lights, lightCount, d_materials, camera
    );
    
    // Copy result back to host
    CUDA_CHECK(cudaMemcpy(framebuffer, d_framebuffer, width * height * sizeof(uint32_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(zbuffer, d_zbuffer, width * height * sizeof(float), cudaMemcpyDeviceToHost));
}

void CudaRenderer::clearBuffers(uint32_t* framebuffer, float* zbuffer, int size) {
    cuda_clear_buffers(d_framebuffer, d_zbuffer, size);
    // Copy cleared buffers back to host if needed
    CUDA_CHECK(cudaMemcpy(framebuffer, d_framebuffer, size * sizeof(uint32_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(zbuffer, d_zbuffer, size * sizeof(float), cudaMemcpyDeviceToHost));
}

void CudaRenderer::renderTestPattern(uint32_t* framebuffer, int width, int height) {
    printf("CudaRenderer::renderTestPattern called with %dx%d\n", width, height);
    
    // Call CUDA test kernel
    cuda_test_pattern(d_framebuffer, width, height);
    
    // Check for CUDA errors
    cudaError_t error = cudaGetLastError();
    if (error != cudaSuccess) {
        printf("CUDA kernel error: %s\n", cudaGetErrorString(error));
        return;
    }
    
    // Copy result back to host
    CUDA_CHECK(cudaMemcpy(framebuffer, d_framebuffer, width * height * sizeof(uint32_t), cudaMemcpyDeviceToHost));
    
    // Check first few pixels to verify data
    printf("First few pixels: %08X %08X %08X %08X\n", 
           framebuffer[0], framebuffer[1], framebuffer[2], framebuffer[3]);
}
