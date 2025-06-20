// Camera/Camera.cpp
#include "Camera.h"
#include "MyMath.h"

void Camera::updateCameraVectors() {
    // 从yaw和pitch计算front向量
    float radYaw = glm::radians(yaw);
    float radPitch = glm::radians(pitch);
    front.x = glm::cos(radPitch) * glm::cos(radYaw);
    front.y = glm::sin(radPitch);
    front.z = glm::cos(radPitch) * glm::sin(radYaw);
    front = glm::normalize(front);

    // 重新计算right和up
    right = glm::normalize(glm::cross(front, worldUp)); // 右向量
    up = glm::normalize(glm::cross(right, front)); // 上向量

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
	tanHalfFovy = tan(glm::radians(fovY) * 0.5f);
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

void Camera::setPosition(const glm::vec3& position) {
	if (this->position != position) {
		this->position = position;
        updateCameraVectors();
		isViewDirty = true;
		isProjectionDirty = true;
	}
}

void Camera::setTarget(const glm::vec3& target) {
	if (this->target != target) {
		this->target = target;
		isViewDirty = true;
		isProjectionDirty = true;
	}
}

// View = Rotate*Translate
glm::mat4 Camera::getViewMatrix() {
    if (!isViewDirty) return viewMatrix;

    glm::vec3 z = glm::normalize(position - target);  // camera forward
    glm::vec3 x = glm::normalize(glm::cross(up, z));        // right
    glm::vec3 y = glm::cross(z, x);                        // up

    glm::mat4 view = glm::mat4(1.0f); // 初始化为单位矩阵

    // 填充视图矩阵
    view[0][0] = x.x; view[1][0] = x.y; view[2][0] = x.z; view[3][0] = -glm::dot(x, position);
    view[0][1] = y.x; view[1][1] = y.y; view[2][1] = y.z; view[3][1] = -glm::dot(y, position);
    view[0][2] = z.x; view[1][2] = z.y; view[2][2] = z.z; view[3][2] = -glm::dot(z, position);
    view[0][3] = 0.0f; view[1][3] = 0.0f; view[2][3] = 0.0f; view[3][3] = 1.0f;


    isViewDirty = false;
    viewMatrix = view;
    return viewMatrix;
	// return glm::lookAt(position, target, up); // debug: 使用glm的lookAt函数生成视图矩阵
}


glm::mat4 Camera::getProjectionMatrix() {
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
            break;
    }
	isProjectionDirty = false;
	return projectionMatrix;
}

// Perspective projection: view space -> clip space -> NDC
glm::mat4 Camera::_getPerspectiveMatrix(){
	float rangeInv = 1.0f / (f - n);

    glm::mat4 persp = glm::mat4(0.0f); // 初始化为零矩阵

    persp[0][0] = 1.0f / (aspect * tanHalfFovy); // 第一列第一行
    persp[1][1] = 1.0f / tanHalfFovy;           // 第二列第二行
    persp[2][2] = -(f + n) * rangeInv;          // 第三列第三行
    persp[2][3] = -1.0f;                        // 第三列第四行
    persp[3][2] = -2.0f * f * n * rangeInv;     // 第四列第三行
    

	return persp;
}

glm::mat4 Camera::_getOrthographicMatrix(){
	float halfWidth = width * 0.5f * zoomFactor;
	float halfHeight = height * 0.5f * zoomFactor;
	
	float l = -halfWidth;
	float r = halfWidth;
	float b = -halfHeight;
	float t = halfHeight;

	glm::mat4 ortho(1.0f);
	ortho[0][0] = 2.0f / (r - l);
	ortho[1][1] = 2.0f / (t - b);
	ortho[2][2] = -2.0f / (f - n);
	ortho[3][0] = -(r + l) / (r - l);
	ortho[3][1] = -(t + b) / (t - b);
	ortho[3][2] = -(f + n) / (n - f);

	return ortho;
	// return glm::ortho(l, r, b, t, n, f);
}

void Camera::reset() {
    position = glm::vec3(0, 0, 5);
	target = glm::vec3(0, 0, 0);
	up = glm::vec3(0, 1, 0);
	fovY = 90.0f;
	tanHalfFovy = tan(glm::radians(fovY) * 0.5f);
	aspect = 1.0f;
	n = 0.1f;
	f = 100.0f;
	width = 1.0f;
	height = 1.0f;
	projectionType = PERSPECTIVE;

	updateCameraVectors();

	isViewDirty = true;
	isProjectionDirty = true;
}

void Camera::processMouseMotion(float dx, float dy) {
    yaw += dx * mouseSensitivity;
    pitch -= dy * mouseSensitivity;

    pitch = glm::clamp(pitch, -89.0f, 89.0f);

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
	glm::vec3 forward = glm::normalize(target - position);
	position += forward * amount;
	target += forward * amount;

	isViewDirty = true;
	isProjectionDirty = true;
}

void Camera::moveRight(float amount) {
	glm::vec3 forward = glm::normalize(target - position);
	glm::vec3 right = glm::normalize(glm::cross(up, forward));
	position += right * amount;
	target += right * amount;

	isViewDirty = true;
	isProjectionDirty = true;
}

void Camera::handleScroll(float scrollY, float deltaTime){
	if (projectionType == PERSPECTIVE) {
		float newFov = fovY + scrollY * 75.f * deltaTime; // 缩放速度可调
		fovY = CLAMP(newFov, 10.0f, 120.0f); // 限制视角范围
		tanHalfFovy = tan(glm::radians(fovY) * 0.5f);
	} else {
		float newZoom = zoomFactor * std::pow(0.25f, scrollY * deltaTime); // 每滚动一步缩放10%
		zoomFactor = CLAMP(newZoom, 0.01f, 100.0f); // 限制缩放因子
	}

	isProjectionDirty = true;
}

Ray Camera::generateRay(int x, int y, int width, int height) const {
    // 将像素坐标转换到 [-1, 1] 的NDC空间
    float ndcX = (x + 0.5f) / width * 2.0f - 1.0f;
    float ndcY = 1.0f - (y + 0.5f) / height * 2.0f; // y反过来

    // 计算屏幕空间坐标
    float aspectRatio = static_cast<float>(width) / height; // 根据传入的宽高计算长宽比
    float px = ndcX * aspectRatio * tanHalfFovy;
    float py = ndcY * tanHalfFovy;

    // 构造 ray direction in world space
    glm::vec3 dir = glm::normalize(px * right + py * up + front);

    return Ray(position, dir);
}

Ray Camera::generateRay(float x, float y, int screenWidth, int screenHeight) const {
    // 将像素坐标 (x, y) 转换为归一化的设备坐标 (NDC) [-1, 1]
    float ndc_x = (2.0f * x / screenWidth) - 1.0f;
    float ndc_y = 1.0f - (2.0f * y / screenHeight); // Y轴通常是反的

    // 转换到相机空间
    // fov 和 aspect_ratio 是相机的属性
	float aspectRatio = static_cast<float>(screenWidth) / screenHeight;
    float camera_x = ndc_x * aspectRatio * tanHalfFovy;
    float camera_y = ndc_y * tanHalfFovy;

    // 计算世界空间中的光线方向
    // position, forward, up, right 是相机在世界空间中的属性
    glm::vec3 dir = glm::normalize(camera_x * right + camera_y * up + front);
    
    return Ray(position, dir);
}


Camera::Camera(){
	reset();
}