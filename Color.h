#pragma once
#include <algorithm>
#include "Math.h"

class Color {
public:
	unsigned char r, g, b;

	Color() : r(0), g(0), b(0) {};
    Color(unsigned char r_, unsigned char g_, unsigned char b_)
        : r(r_), g(g_), b(b_) {}

    static Color fromFloat(float r, float g, float b) {
        return Color(
            static_cast<unsigned char>(MIN(255.0f, r * 255.0f)),
            static_cast<unsigned char>(MIN(255.0f, g * 255.0f)),
            static_cast<unsigned char>(MIN(255.0f, b * 255.0f))
        );
    }
    static Color fromVec(const Vec3& v) {
        return Color(
            static_cast<unsigned char>(MIN(255.0f, v.x * 255.0f)),
            static_cast<unsigned char>(MIN(255.0f, v.y * 255.0f)),
            static_cast<unsigned char>(MIN(255.0f, v.z * 255.0f))
        );
    }

    static uint32_t ColorToUint32(const Color& c) {
        return (0xFF << 24)    // Alpha (Ä¬ÈÏ²»Í¸Ã÷ 0xFF)
            | (c.r << 16)      // Red
            | (c.g << 8)       // Green
            | c.b;             // Blue
    }

    static Color Uint32ToColor(uint32_t pixel) {
        return Color{
            static_cast<unsigned char>((pixel >> 16) & 0xFF), // Red
            static_cast<unsigned char>((pixel >> 8) & 0xFF), // Green
            static_cast<unsigned char>(pixel & 0xFF)          // Blue
        };
    }

    
};