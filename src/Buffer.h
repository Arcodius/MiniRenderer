#pragma once

#include "Color.h"
#include <vector>

template <typename PixelType>
class Buffer {
public:
	int width, height;
	std::vector<PixelType> pixels;

	Buffer() : width(0), height(0) {}
	Buffer(int w, int h) : width(w), height(h), pixels(w * h) {}

	void clear(const PixelType& c) {
		std::fill(pixels.begin(), pixels.end(), c);
	}

	void setPixel(int x, int y, const PixelType& c) {
		if (0 <= x && x < width && 0 <= y && y < height)
			pixels[y * width + x] = c;
	}
	const PixelType* data() const {
		return pixels.data();
	}

	PixelType& operator[](int index) {
		return pixels[index];
	}

	const PixelType& operator[](int idx) const {
		return pixels[idx];
	}

	PixelType& operator()(int x, int y) {
		return pixels[y * width + x];
	}

	const PixelType& operator()(int x, int y) const {
		return pixels[y * width + x];
	}
};