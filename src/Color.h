#pragma once
#include <algorithm>
#include "MyMath.h"

class Color {
public:
    static uint32_t VecToUint32(const glm::vec3& v) {
		return (0xFF << 24)    // Alpha 
			| (static_cast<uint32_t>(v.x * 255.0f) << 16) // Red
			| (static_cast<uint32_t>(v.y * 255.0f) << 8)  // Green
			| static_cast<uint32_t>(v.z * 255.0f);         // Blue
	}

    static glm::vec3 Uint32ToVec(uint32_t pixel) {
		return glm::vec3{
			static_cast<float>((pixel >> 16) & 0xFF) / 255.0f, // Red
			static_cast<float>((pixel >> 8) & 0xFF) / 255.0f,  // Green
			static_cast<float>(pixel & 0xFF) / 255.0f          // Blue
		};
	}
};