#include "ObjLoader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

bool ObjLoader::LoadFromFile(const std::string& filename, Mesh& outMesh) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

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
            std::string token;
            for (int i = 0; i < 3; ++i) {
                iss >> token;
                if (uniqueVertexMap.count(token)) {
                    outMesh.indices.push_back(uniqueVertexMap[token]);
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
                outMesh.indices.push_back(newIdx);
                uniqueVertexMap[token] = newIdx;
            }
        }
    }

    return true;
}