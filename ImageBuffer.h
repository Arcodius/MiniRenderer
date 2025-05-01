#pragma once

#include "Color.h"
#include <vector>

class ImageBuffer {
public:
	int width, height;
	std::vector<Color> pixels;

	ImageBuffer(int w, int h) : width(w), height(h), pixels(w * h) {}

	void clear(const Color& c) {
		std::fill(pixels.begin(), pixels.end(), c);
	}

	void setPixel(int x, int y, const Color& c) {
		if (0 <= x && x < width && 0 <= y && y < height)
			pixels[y * width + x] = c;
	}
	const Color *data() const {
		return pixels.data();
	}
};