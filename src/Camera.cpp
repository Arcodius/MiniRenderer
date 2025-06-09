// Camera/Camera.cpp
#include "Camera.h"
#include "Math.h"

void Camera::updateCameraVectors() {
    // 从yaw和pitch计算front向量
    float radYaw = deg2rad(yaw);
    float radPitch = deg2rad(pitch);
    front.x = cos(radPitch) * cos(radYaw);
    front.y = sin(radPitch);
    front.z = cos(radPitch) * sin(radYaw);
    front = front.normalized();

    // 重新计算right和up
    right = front.cross(worldUp).normalized();
    up = right.cross(front).normalized();

    // target保持与front一致
    target = position + front;
    isViewDirty = true;
}


void Camera::setPerspective(bool option) {
	ProjectionType type = option ? PERSPECTIVE : ORTHOGRAPHIC;
	if (this->projectionType != type) {
		this->projectionType = type;
		isProjectionDirty = true;
	}
}

// Aspect = width / height
void Camera::setAspect(float aspect) {
	if (this->aspect != aspect) {
		this->aspect = aspect;
		isProjectionDirty = true;
	}
	width = aspect;  // height = 1.0f
}

void Camera::setFovY(float fovY) {
	if (this->fovY != fovY) {
		this->fovY = fovY;
		isProjectionDirty = true;
	}
}

void Camera::setNearClip(float nearClip) {
	if (this->n != nearClip) {
		this->n = nearClip;
		isProjectionDirty = true;
	}
}

void Camera::setFarClip(float farClip) {
	if (this->f != farClip) {
		this->f = farClip;
		isProjectionDirty = true;
	}
}

void Camera::setPosition(const Vec3& position) {
	if (this->position != position) {
		this->position = position;
        updateCameraVectors();
		isViewDirty = true;
		isProjectionDirty = true;
	}
}

void Camera::setTarget(const Vec3& target) {
	if (this->target != target) {
		this->target = target;
		isViewDirty = true;
		isProjectionDirty = true;
	}
}

// View = Rotate*Translate
Mat4 Camera::getViewMatrix() {
    if (!isViewDirty) return viewMatrix;

    Vec3 z = (position - target).normalized();  // camera forward
    Vec3 x = up.cross(z).normalized();          // right
    Vec3 y = z.cross(x);                        // up

    Mat4 view = Mat4::identity();

    // rotation
    view[0][0] = x.x; view[0][1] = y.x; view[0][2] = z.x;
    view[1][0] = x.y; view[1][1] = y.y; view[1][2] = z.y;
    view[2][0] = x.z; view[2][1] = y.z; view[2][2] = z.z;
    // translation
    view[3][0] = -x.dot(position);
    view[3][1] = -y.dot(position);
    view[3][2] = -z.dot(position);
    view[3][3] = 1.0f;

    isViewDirty = false;
    viewMatrix = view;
    return viewMatrix;
}


Mat4 Camera::getProjectionMatrix() {
	if (!isProjectionDirty) {
		return projectionMatrix;
	}
    switch (projectionType) {
        case PERSPECTIVE:
		    projectionMatrix = _getPerspectiveMatrix();
            break;
        case ORTHOGRAPHIC:
			projectionMatrix = _getOrthographicMatrix();
            break;
        default:
            throw std::runtime_error("Unknown projection type");
            break;
    }
	isProjectionDirty = false;
	return projectionMatrix;
}

// Perspective projection: view space -> clip space -> NDC
Mat4 Camera::_getPerspectiveMatrix(){
    float tanHalfFovy = 1.0f / tan(RADIAN(fovY / 2.0f));
	float rangeInv = 1.0f / (n - f);

    Mat4 persp = Mat4::identity();
    persp[0][0] = 1.0f / (aspect * tanHalfFovy);
    persp[1][1] = 1.0f / tanHalfFovy;
    persp[2][2] = -(f + n) / (f - n);
    persp[2][3] = -1.0f;
    persp[3][2] = -(2.0f * f * n) / (f - n);
    persp[3][3] = 0.0f;

	return persp;

}

Mat4 Camera::_getOrthographicMatrix(){
	float halfWidth = width * 0.5f * zoomFactor;
	float halfHeight = height * 0.5f * zoomFactor;

	float l = position.x - halfWidth;
	float r = position.x + halfWidth;
	float b = position.y - halfHeight;
	float t = position.y + halfHeight;

	Mat4 ortho;
	ortho[0][0] = 2.0f / (r - l);
	ortho[1][1] = 2.0f / (t - b);
	ortho[2][2] = -2.0f / (f - n);
	ortho[3][0] = -(r + l) / (r - l);
	ortho[3][1] = -(t + b) / (t - b);
	ortho[3][2] = -(f + n) / (f - n);
	ortho[3][3] = 1.0f;

	return ortho;
}

void Camera::reset() {
    position = Vec3(0, 0, 5);
	target = Vec3(0, 0, 0);
	up = Vec3(0, 1, 0);
	fovY = 90.0f;
	aspect = 1.0f;
	n = 0.1f;
	f = 100.0f;
	width = 1.0f;
	height = 1.0f;

	isViewDirty = true;
	isProjectionDirty = true;
}

void Camera::processMouseMotion(float dx, float dy) {
    yaw += dx * mouseSensitivity;
    pitch -= dy * mouseSensitivity;

    pitch = CLAMP(pitch, -89.0f, 89.0f);

    updateCameraVectors();
}

void Camera::handleKeyPress(char wParam, float deltaTime) {
	const float amount = moveSpeed * deltaTime;
    if (wParam == 'w' || wParam == 'W') moveForward(amount);
    else if (wParam == 's' || wParam == 'S') moveForward(-amount);
    else if (wParam == 'a' || wParam == 'A') moveRight(amount);
    else if (wParam == 'd' || wParam == 'D') moveRight(-amount);
}

void Camera::moveForward(float amount) {
	Vec3 forward = (target - position).normalized();
	position += forward * amount;
	target += forward * amount;

	isViewDirty = true;
	isProjectionDirty = true;
}

void Camera::moveRight(float amount) {
	Vec3 forward = (target - position).normalized();
	Vec3 right = up.cross(forward).normalized();
	position += right * amount;
	target += right * amount;

	isViewDirty = true;
	isProjectionDirty = true;
}

// Rotate the camera around the target point
void Camera::rotate(float yaw, float pitch) {
	Vec3 forward = (target - position).normalized();
	float radius = (target - position).length();

	float theta = atan2(forward.z, forward.x);
	float phi = asin(forward.y);

	theta += yaw;
	phi += pitch;
	phi = CLAMP(phi, -PI / 2 + 0.1f, PI / 2 - 0.1f);

	forward.x = cos(phi) * cos(theta);
	forward.y = sin(phi);
	forward.z = cos(phi) * sin(theta);

	target = position + forward * radius;

	isViewDirty = true;
	isProjectionDirty = true;
}