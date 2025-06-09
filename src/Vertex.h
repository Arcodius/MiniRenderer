#pragma once
#include "MyMath.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;

	Vertex() : position(0, 0, 0), normal(0, 0, 0), uv(0, 0) {}
	Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex)
		: position(pos), normal(norm), uv(tex) {}
};