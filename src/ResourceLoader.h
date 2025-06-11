#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "Mesh.h"

class ResourceLoader {
private:
    ResourceLoader() = default; // Prevent instantiation
    ~ResourceLoader() = default; // Prevent instantiation

    // Disable copy and move semantics
    ResourceLoader(const ResourceLoader&) = delete;
    ResourceLoader& operator=(const ResourceLoader&) = delete;
    ResourceLoader(ResourceLoader&&) = delete;
    ResourceLoader& operator=(ResourceLoader&&) = delete;

    std::filesystem::path getExecutabePath();
public:
	static bool loadMeshFromFile(const std::string& filename, Mesh& outMesh);
    static std::vector<uint32_t> loadTextureFromFile(const std::string& path, int& texWidth, int& texHeight);
};