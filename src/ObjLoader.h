#pragma once
#include <filesystem>
#include <string>
#include "Mesh.h"

class ObjLoader {
private:
    ObjLoader() = default; // Prevent instantiation
    ~ObjLoader() = default; // Prevent instantiation

    // Disable copy and move semantics
    ObjLoader(const ObjLoader&) = delete;
    ObjLoader& operator=(const ObjLoader&) = delete;
    ObjLoader(ObjLoader&&) = delete;
    ObjLoader& operator=(ObjLoader&&) = delete;

    std::filesystem::path getExecutabePath();
public:
	static bool LoadFromFile(const std::string& filename, Mesh& outMesh);
};