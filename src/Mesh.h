#pragma once
#include <string>
#include <vector>
#include "Vertex.h"

class Mesh {
private:
    std::string name;
public:
    
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

    std::string getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }

	void clear();
};