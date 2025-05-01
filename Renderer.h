#pragma once

#include <Windows.h>

#include "Scene.h"

class Renderer {
public:
	int screenWidth, screenHeight;

	Renderer(int width, int height) : screenWidth(width), screenHeight(height) {}

	void render(HDC hdc, Scene scene);
};