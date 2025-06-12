#pragma once
#include "MyMath.h"

struct Vertex {
	glm::vec3 localPos; // 局部空间位置
	glm::vec3 worldPos; // 世界空间位置
	glm::vec3 normal;
	glm::vec2 uv;

	Vertex() : localPos(0), worldPos(0), normal(0), uv(0){}
	Vertex(const glm::vec3& localPos, const glm::vec3& worldPos, const glm::vec3& normal, const glm::vec2& uv)
		: localPos(localPos), worldPos(worldPos), normal(normal), uv(uv){}
};