#pragma once
#include "Math.h"

struct Vertex {
	Vec3 position;
	Vec3 normal;
	Vec2 texCoord;

	Vertex() : position(0, 0, 0), normal(0, 0, 0), texCoord(0, 0) {}
	Vertex(const Vec3& pos, const Vec3& norm, const Vec2& tex)
		: position(pos), normal(norm), texCoord(tex) {}
};