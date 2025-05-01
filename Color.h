#pragma once
#include <algorithm>

class Color {
public:
	unsigned char r, g, b;

	Color() : r(0), g(0), b(0) {};
    Color(unsigned char r_, unsigned char g_, unsigned char b_)
        : r(r_), g(g_), b(b_) {}

    static Color fromFloat(float r, float g, float b) {
        return Color(
            static_cast<unsigned char>(min(255.0f, r * 255.0f)),
            static_cast<unsigned char>(min(255.0f, g * 255.0f)),
            static_cast<unsigned char>(min(255.0f, b * 255.0f))
        );
    }

};