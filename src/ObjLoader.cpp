#include "ObjLoader.h"

#include <SDL3/SDL_log.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <windows.h>

std::filesystem::path get_executable_path() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
}

bool ObjLoader::LoadFromFile(const std::string& filename, Mesh& outMesh) {
    std::filesystem::path exe_dir = get_executable_path();
    std::filesystem::path abs_path = exe_dir / filename;
    std::ifstream file(abs_path);
    if (!file.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open file: %s", abs_path.c_str());
        return false;
    }
    

    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<Vec2> texcoords;
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

                Vec3 pos = positions[vIdx];
                Vec2 tex = tIdx >= 0 ? texcoords[tIdx] : Vec2(0, 0);
                Vec3 norm = nIdx >= 0 ? normals[nIdx] : Vec3(0, 0, 1);

                Vertex vert(pos, norm, tex);
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