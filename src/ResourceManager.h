#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "Mesh.h"

class ResourceManager {
private:
    ResourceManager() = default; // Prevent instantiation
    ~ResourceManager() = default; // Prevent instantiation

    // Disable copy and move semantics
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    std::filesystem::path getExecutabePath();
public:
	static bool loadMeshFromFile(const std::string& filename, Mesh& outMesh);
    static std::vector<uint32_t> loadTextureFromFile(const std::string& path, int& texWidth, int& texHeight);
    static void saveFramebufferToBMP(const std::string& filename, const std::vector<uint32_t>& framebuffer, int width, int height);
};