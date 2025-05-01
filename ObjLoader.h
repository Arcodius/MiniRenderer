#pragma once
#include <string>
#include "Mesh.h"

class ObjLoader {
public:
	static bool LoadFromFile(const std::string& filename, Mesh& outMesh);
};