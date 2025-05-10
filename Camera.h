#pragma once
#include "Math.h"


class Camera {
private:
    Vec3 position;
    Vec3 target;
    Vec3 up;
    float fovY;      // in degrees
    float aspect;    // width / height

    float n;
    float f;
    float width;
    float height;
    float zoomFactor = 1.0f;

    Mat4 viewMatrix;
    Mat4 projectionMatrix;
    bool isViewDirty = true;
    bool isProjectionDirty = true;

    float moveSpeed = 0.5f;

    Mat4 _getPerspectiveMatrix();
	Mat4 _getOrthographicMatrix();

public:
    enum ProjectionType {
        PERSPECTIVE,
        ORTHOGRAPHIC
    };
    ProjectionType projectionType;
    

    Camera()
        : position(0, 0, 5), target(0, 0, 0), up(0, 1, 0), fovY(45.0f), aspect(1.0f), 
        n(0.1f), f(100.0f), width(1.0f), height(1.0f), projectionType(PERSPECTIVE) {}

    void reset();
    Vec3 getPosition() const { return position; }
    Vec3 getTarget() const { return target; }
    Vec3 getUp() const { return up; }
    float getFovY() const { return fovY; }
    float getAspect() const { return aspect; }
    float getNearClip() const { return n; }
    float getFarClip() const { return f; }
    Mat4 getViewMatrix();
    Mat4 getProjectionMatrix();
    // TODO: set attributes -> update projection matrix -> update scene
    void setPosition(const Vec3& position);
    void setTarget(const Vec3& target);
	void setAspect(float aspect);
	void setFovY(float fovY);
	void setNearClip(float nearClip);
	void setFarClip(float farClip);
    void setPerspective(bool option);

    void handleKeyPress(char wParam, float deltaTime);
    void moveForward(float amount);
    void moveRight(float amount);
    void rotate(float yaw, float pitch);
};