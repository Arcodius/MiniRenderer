#pragma once
#include "Math.h"

class Camera {
public:
    Vec3 position;
    Vec3 target;
    Vec3 up;
    float fovY;      // in degrees
    float aspect;    // width / height
    float nearClip;
    float farClip;

    Camera()
        : position(0, 0, 5), target(0, 0, 0), up(0, 1, 0),
        fovY(90.0f), aspect(1.0f), nearClip(0.1f), farClip(100.0f) {}

    void reset();
    Mat4 getViewMatrix() const;
    Mat4 getProjectionMatrix() const;
};