#pragma once
#include "MyMath.h"

struct Line
{
    glm::vec3 v1;
    glm::vec3 v2;
    bool isNull = false;

    Line() :isNull(true){}
    Line(const glm::vec3& v1, const glm::vec3& v2) 
        : v1(v1), v2(v2), isNull(false) {}
};