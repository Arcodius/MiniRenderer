#pragma once

#include <Windows.h>

#include "Buffer.h"

class Scene;

class Renderer {
public:
	int screenWidth, screenHeight;
	Buffer<uint32_t> framebuffer;
	Buffer<float> depthbuffer;
private:
	static bool _isBackFacing(const Vec3& v0, const Vec3& v1, const Vec3& v2);
	void _drawFilledTriangle(Vec3 v0, Vec3 v1, Vec3 v2, uint32_t color, int w, int h);
	void _presentToHDC(HDC hdc, int w, int h);

public:
	Renderer(int width, int height) : screenWidth(width), screenHeight(height) {
		framebuffer = Buffer<uint32_t>(width, height);
		depthbuffer = Buffer<float>(width, height);
		clearBuffers();
	}

	void clearBuffers();
	void render(HDC hdc, Scene scene);
	
};