#pragma once

#include "MyMath.h"
#include "Ray.h"

class Camera {
private:
    glm::vec3 position;
    glm::vec3 target;
    
    float yaw = -90.0f;   // 初始朝向为-z
    float pitch = 0.0f;   // 初始无俯仰
    glm::vec3 front = glm::vec3(0, 0, -1); // 相机朝向
    glm::vec3 right;
    glm::vec3 up;              // 相机上方向
    glm::vec3 worldUp = glm::vec3(0, 1, 0);

    float fovY;      // in degrees
    float aspect;    // width / height

    float n;
    float f;
    float width;
    float height;
    float zoomFactor = 5.f;

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    bool isViewDirty = true;
    bool isProjectionDirty = true;

    float moveSpeed = 2.0f;
    float mouseSensitivity = 0.1f;

    glm::mat4 _getPerspectiveMatrix();
	glm::mat4 _getOrthographicMatrix();

    void updateCameraVectors();

public:
    enum ProjectionType {
        PERSPECTIVE,
        ORTHOGRAPHIC
    };
    ProjectionType projectionType;
    

    Camera();

    void reset();
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getTarget() const { return target; }
    glm::vec3 getUp() const { return up; }
    float getFovY() const { return fovY; }
    float getAspect() const { return aspect; }
    float getNearClip() const { return n; }
    float getFarClip() const { return f; }
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();
    // TODO: set attributes -> update projection matrix -> update scene
    void setPosition(const glm::vec3& position);
    void setTarget(const glm::vec3& target);
	void setAspect(float aspect);
	void setFovY(float fovY);
	void setNearClip(float nearClip);
	void setFarClip(float farClip);
    void setPerspective(bool option);

    void processMouseMotion(float dx, float dy);
    void handleKeyPress(char wParam, float deltaTime);
    void moveForward(float amount);
    void moveRight(float amount);
    void handleScroll(float scrollY, float deltaTime);

    // Light tracing
    Ray generateRay(int x, int y, int width, int height) const;

};