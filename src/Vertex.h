#pragma once
#include "MyMath.h"

struct Vertex {
	glm::vec3 localPos; // 局部空间位置
	glm::vec3 normal;
	glm::vec2 uv;

	Vertex() : localPos(0), normal(0), uv(0){}
	Vertex(const glm::vec3& localPos, const glm::vec3& worldPos, const glm::vec3& normal, const glm::vec2& uv)
		: localPos(localPos), normal(normal), uv(uv){}
};

// A structure to hold a vertex and its corresponding clip-space position
struct ClippedVertex {
    Vertex vertex;
    glm::vec4 clipPos;
};

struct VertexShaderOutput {
    glm::vec4 clipPos;     // 用于裁剪与 NDC 映射
    glm::vec3 worldPos;    // 世界坐标用于 shading
    glm::vec3 normal;      // 世界空间 normal
    glm::vec2 uv;          // 可选：纹理坐标
    float w;               // 原始 clipPos.w，用于透视校正插值
};

