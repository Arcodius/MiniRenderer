// Camera/Camera.cpp
#include "Camera.h"
#include "Math.h"

Mat4 Camera::getViewMatrix() const {
    Vec3 zAxis = (position - target).normalized(); // camera direction
    Vec3 xAxis = up.cross(zAxis).normalized();     // camera right
    Vec3 yAxis = zAxis.cross(xAxis);               // camera up

    Mat4 view = Mat4();
    view[0][0] = xAxis.x; view[0][1] = yAxis.x; view[0][2] = zAxis.x;
    view[1][0] = xAxis.y; view[1][1] = yAxis.y; view[1][2] = zAxis.y;
    view[2][0] = xAxis.z; view[2][1] = yAxis.z; view[2][2] = zAxis.z;
    view[3][0] = -xAxis.dot(position);
    view[3][1] = -yAxis.dot(position);
    view[3][2] = -zAxis.dot(position);

    return view;
}

Mat4 Camera::getProjectionMatrix() const {
    float f = 1.0f / tanf(fovY * 0.5f * 3.14159f / 180.0f);
    Mat4 proj = Mat4::zero();

    proj[0][0] = f / aspect;
    proj[1][1] = f;
    proj[2][2] = (farClip + nearClip) / (nearClip - farClip);
    proj[2][3] = -1.0f;
    proj[3][2] = (2 * farClip * nearClip) / (nearClip - farClip);

    return proj;
}

void Camera::reset() {
    position = Vec3(0, 0, 5);
	target = Vec3(0, 0, 0);
	up = Vec3(0, 1, 0);
	fovY = 90.0f;
	aspect = 1.0f;
	nearClip = 0.1f;
	farClip = 100.0f;
}
