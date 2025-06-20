#include "ResourceManager.h"

#include <SDL3/SDL_log.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <windows.h>



#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::filesystem::path get_executable_path() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
}

bool ResourceManager::loadMeshFromFile(const std::string& filename, Mesh& outMesh) {
    std::filesystem::path exe_dir = get_executable_path();
    std::filesystem::path abs_path = exe_dir / filename;
    std::ifstream file(abs_path);
    if (!file.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open file: %s", abs_path.c_str());
        return false;
    }
    

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::unordered_map<std::string, unsigned int> uniqueVertexMap;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            float x, y, z; iss >> x >> y >> z;
            positions.emplace_back(x, y, z);
        }
        else if (prefix == "vt") {
            float u, v; iss >> u >> v;
            texcoords.emplace_back(u, v);
        }
        else if (prefix == "vn") {
            float x, y, z; iss >> x >> y >> z;
            normals.emplace_back(x, y, z);
        }
        else if (prefix == "f") {
            std::vector<unsigned int> faceIndices;
            std::string token;

            while (iss >> token) {
                if (uniqueVertexMap.count(token)) {
                    faceIndices.push_back(uniqueVertexMap[token]);
                    continue;
                }

                std::istringstream vss(token);
                std::string vi, ti, ni;
                std::getline(vss, vi, '/');
                std::getline(vss, ti, '/');
                std::getline(vss, ni, '/');

                int vIdx = std::stoi(vi) - 1;
                int tIdx = ti.empty() ? -1 : std::stoi(ti) - 1;
                int nIdx = ni.empty() ? -1 : std::stoi(ni) - 1;

                glm::vec3 pos = positions[vIdx];
                glm::vec2 tex = tIdx >= 0 ? texcoords[tIdx] : glm::vec2(0, 0);
                glm::vec3 norm = nIdx >= 0 ? normals[nIdx] : glm::vec3(0, 1, 0);

                Vertex vert = Vertex();
                vert.localPos = pos;
                vert.uv = tex;
                vert.normal = norm;
                // printf("Loaded vertex: pos=(%.2f, %.2f, %.2f), uv=(%.2f, %.2f), norm=(%.2f, %.2f, %.2f)\n", pos.x, pos.y, pos.z, tex.x, tex.y, norm.x, norm.y, norm.z);
                outMesh.vertices.push_back(vert);
                unsigned int newIdx = (unsigned int)(outMesh.vertices.size() - 1);
                faceIndices.push_back(newIdx);
                uniqueVertexMap[token] = newIdx;
            }

            // triangle fan: (v0,v1,v2), (v0,v2,v3), ..., (v0,vn-2,vn-1)
            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                outMesh.indices.push_back(faceIndices[0]);
                outMesh.indices.push_back(faceIndices[i]);
                outMesh.indices.push_back(faceIndices[i + 1]);
            }
        }
    }
    outMesh.setName(filename);
    file.close();
    return true;
}

std::vector<uint32_t> ResourceManager::loadTextureFromFile(const std::string& path, int& texWidth, int& texHeight) {
    int channels;
    unsigned char* data = stbi_load(path.c_str(), &texWidth, &texHeight, &channels, 4); // Force 4 channels (RGBA)
    if (!data) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture: %s", path.c_str());
        return {};
    }
    std::vector<uint32_t> textureData(texWidth * texHeight);
    memcpy(textureData.data(), data, texWidth * texHeight * 4); // Copy texture data
    stbi_image_free(data);
    return textureData;
}

// Function to save the framebuffer as a BMP image
void ResourceManager::saveFramebufferToBMP(const std::string& filename, const Buffer<uint32_t>& framebuffer) {
    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        SDL_Log("Failed to open file for writing: %s", filename.c_str());
        return;
    }

    int width = framebuffer.width;
    int height = framebuffer.height;

    // BMP Header
    uint8_t header[54] = {
        0x42, 0x4D,           // Signature
        0, 0, 0, 0,           // File size (will be filled later)
        0, 0,                 // Reserved
        0, 0,                 // Reserved
        54, 0, 0, 0,          // Offset to pixel data
        40, 0, 0, 0,          // DIB header size
        0, 0, 0, 0,           // Width (will be filled later)
        0, 0, 0, 0,           // Height (will be filled later)
        1, 0,                 // Planes
        32, 0,                // Bits per pixel
        0, 0, 0, 0,           // Compression
        0, 0, 0, 0,           // Image size (can be 0 for uncompressed)
        0, 0, 0, 0,           // X pixels per meter
        0, 0, 0, 0,           // Y pixels per meter
        0, 0, 0, 0,           // Colors in color table
        0, 0, 0, 0            // Important color count
    };

    // Fill width and height
    *reinterpret_cast<int32_t*>(&header[18]) = width;
    *reinterpret_cast<int32_t*>(&header[22]) = -height; // Negative height for top-down BMP

    // Fill file size
    int fileSize = 54 + width * height * 4;
    *reinterpret_cast<int32_t*>(&header[2]) = fileSize;

    // Write header
    file.write(reinterpret_cast<char*>(header), sizeof(header));

    // Write pixel data directly from framebuffer
    file.write(reinterpret_cast<const char*>(framebuffer.data()), width * height * 4);

    file.close();
    SDL_Log("Framebuffer saved to %s", filename.c_str());
}
