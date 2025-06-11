#pragma once
#include <algorithm>
#include "MyMath.h"

class Color: public glm::vec3 {
public:
	unsigned char r, g, b;

	Color() : r(0), g(0), b(0) {};
    Color(unsigned char r_, unsigned char g_, unsigned char b_)
        : r(r_), g(g_), b(b_) {}
    Color(glm::vec3 v) {
        r = static_cast<unsigned char>((std::min)(255.0f, v.x * 255.0f));
        g = static_cast<unsigned char>((std::min)(255.0f, v.y * 255.0f));
        b = static_cast<unsigned char>((std::min)(255.0f, v.z * 255.0f));
    }

    static Color fromFloat(float r, float g, float b) {
        return Color(
            static_cast<unsigned char>((std::min)(255.0f, r * 255.0f)),
            static_cast<unsigned char>((std::min)(255.0f, g * 255.0f)),
            static_cast<unsigned char>((std::min)(255.0f, b * 255.0f))
        );
    }
    static Color fromVec(const glm::vec3& v) {
        return Color(
            static_cast<unsigned char>((std::min)(255.0f, v.x * 255.0f)),
            static_cast<unsigned char>((std::min)(255.0f, v.y * 255.0f)),
            static_cast<unsigned char>((std::min)(255.0f, v.z * 255.0f))
        );
    }

    static uint32_t ColorToUint32(const Color& c) {
        return (0xFF << 24)    // Alpha (Ĭ�ϲ�͸�� 0xFF)
            | (c.r << 16)      // Red
            | (c.g << 8)       // Green
            | c.b;             // Blue
    }

    // Convert from uint32_t pixel format (RGB) to Color(255)
    static Color Uint32ToColor(uint32_t pixel) {
        return Color{
            static_cast<unsigned char>((pixel >> 16) & 0xFF), // Red
            static_cast<unsigned char>((pixel >> 8) & 0xFF), // Green
            static_cast<unsigned char>(pixel & 0xFF)          // Blue
        };
    }

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