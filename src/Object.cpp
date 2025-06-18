#include "Object.h"

#include <SDL3/SDL_log.h>

#include "MyMath.h"
#include "Ray.h"
#include "ResourceLoader.h"
#include "Intersection.h"

void Object::update() {
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0, 0, 1)) *
                               glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0, 1, 0)) *
                               glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0));
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
    matrix = translationMatrix * rotationMatrix * scaleMatrix;
}


void Object::translate(const glm::vec3& translation) {
	delta_position = translation;
	update();
}

void Object::rotate(const glm::vec3& rotationDelta) {
	delta_rotation = rotationDelta;
	update();
}

void Object::resize(const glm::vec3& scaleDelta) {
	delta_scale = scaleDelta;
	update();
}

void Object::setPosition(const glm::vec3& newPosition) {
	position = newPosition;
	update();
}

void Object::setRotation(const glm::vec3& newRotation) {
	rotation = newRotation;
	update();
}

void Object::setScale(const glm::vec3& newScale) {
	scale = newScale;
	update();
}

void Object::setMesh(const std::string& meshPath) {
	Mesh new_mesh;
	bool success = ResourceLoader::loadMeshFromFile(meshPath, new_mesh);
	if (!success) {
        SDL_Log("Failed to load mesh from file: %s", meshPath.c_str());
		return;
	}
	mesh = new_mesh;
	update();
}

void Object::setAsPrimitive(Object::PrimitiveType PrimitiveType) {
	isPrimitive = true;
	primitive = PrimitiveType;
	// Set default mesh based on primitive type
	switch (PrimitiveType) {
		case Object::PLANE:
			setMesh("Resources\\plane.obj");
			break;
		case Object::CUBE:
			setMesh("Resources\\cube.obj");
			break;
		case Object::SPHERE:
			setMesh("Resources\\sphere.obj");
			break;
		case Object::CYLINDER:
			setMesh("Resources\\cylinder.obj");
			break;
		case Object::CONE:
			setMesh("Resources\\cone.obj");
			break;
		case Object::TORUS:
			setMesh("Resources\\torus.obj");
			break;
	}
}

void Object::updateMesh() {
    update();
    mesh.triangles.clear();
    glm::mat4 modelMatrix = getMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        TransformedVertex transformedVertics[3];
        for (int j = 0; j < 3; ++j) {
            glm::vec3 worldPos = modelMatrix * glm::vec4(mesh.vertices[mesh.indices[i + j]].localPos, 1.0f);
            glm::vec3 worldNormal = glm::normalize(normalMatrix * mesh.vertices[mesh.indices[i + j]].normal);
            transformedVertics[j] = TransformedVertex(worldPos, worldNormal, mesh.vertices[mesh.indices[i + j]].uv);
        }
        mesh.triangles.push_back(Triangle(transformedVertics[0], transformedVertics[1], transformedVertics[2], material));
    }
}

// 逐三角形求交
bool GenericObject::intersect(const Ray& ray, Intersection& isect) const {
    return mesh.intersect(ray, isect, material);
}

void Sphere::updateRadius() {
    // Update the mesh to match the sphere's radius
    scale = glm::vec3(radius);
    update();
}

bool Sphere::intersect(const Ray& ray, Intersection& isect) const {
    glm::vec3 oc = ray.origin - getPosition();
    float r = radius;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - r * r;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0.0f) {
		return false;
	}

    float sqrtDisc = std::sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);

    float t = -1.0f;
    // 找到最小的正数解 t
    if (t1 > 0.001f && t2 > 0.001f) {
        t = glm::min(t1, t2);
    } else if (t1 > 0.001f) {
        t = t1;
    } else if (t2 > 0.001f) {
        t = t2;
    } else {
        return false; // 两个交点都在光线起点之后或非常近，无效
    }

    // 只需要检查t是否有效
    if (t < 0.001f || t >= isect.t) {
		return false; // 光线方向错误或交点更远
	} 

    // 成功命中
    isect.t = t;
    isect.position = ray.origin + t * ray.direction;
    isect.normal = glm::normalize(isect.position - getPosition());
    isect.material = getMaterial();
    isect.hit = true;
    return true;
}

void Plane::updateRotation() {
    normal = glm::normalize(normal);

    // 默认平面的法线方向是 (0, 1, 0)，即世界Y轴
    glm::vec3 defaultNormal(0.0f, 1.0f, 0.0f);

    // 如果 normal 与 defaultNormal 非共线，求旋转
    if (glm::dot(normal, defaultNormal) < 0.999f) {
        // 计算旋转四元数
        glm::quat q = glm::rotation(defaultNormal, normal);

        // 转为欧拉角
        rotation = glm::eulerAngles(q); // rotation 是 glm::vec3 (radians)
    } else {
        // 如果已经对齐或几乎对齐，无需旋转
        rotation = glm::vec3(0.0f);
    }
}

bool Plane::intersect(const Ray& ray, Intersection& isect) const {
    float d = -glm::dot(normal, position); // 平面方程的常数项

    float denom = glm::dot(normal, ray.direction);
    if (std::abs(denom) < 1e-6) return false; // 光线与平面平行

    if (denom > 0.0f) return false; // 光线方向与平面法线方向相反

    float t = -(glm::dot(normal, ray.origin) + d) / denom;
    if (t < 0.001f || t >= isect.t) return false; // 光线方向错误或交点更远

    glm::vec3 hitPoint = ray.origin + t * ray.direction;

    if (finite) {
        // 计算交点位置
        glm::vec3 localPoint = hitPoint - position; // 转换到局部空间

        // 根据平面的法线方向，确定边界检查的轴
        glm::vec3 absNormal = glm::abs(normal);
        float halfWidth = scale.x; // 平面的宽度的一半，但一般为2*2平面，故不做修改
        float halfHeight = scale.z; // 平面的高度的一半
        
        if (absNormal.x > 0.99f) { // 平面垂直X轴，XY为边界轴
            if (localPoint.y < -halfHeight || localPoint.y > halfHeight ||
                localPoint.z < -halfWidth  || localPoint.z > halfWidth) {
                return false;
            }
        } else if (absNormal.y > 0.99f) { // 平面垂直Y轴，XZ为边界轴
            if (localPoint.x < -halfWidth || localPoint.x > halfWidth ||
                localPoint.z < -halfHeight || localPoint.z > halfHeight) {
                return false;
            }
        } else if (absNormal.z > 0.99f) { // 平面垂直Z轴，XY为边界轴
            if (localPoint.x < -halfWidth || localPoint.x > halfWidth ||
                localPoint.y < -halfHeight || localPoint.y > halfHeight) {
                return false;
            }
        }

        isect.position = hitPoint;
    } else {
        isect.position = hitPoint;
    }

    isect.t = t;
    isect.normal = normal;
    isect.material = getMaterial();
    isect.hit = true;

    return true;
}

bool Cube::intersect(const Ray& ray, Intersection& isect) const {
    glm::vec3 minBound = position - scale * 0.5f; // 立方体的最小边界
    glm::vec3 maxBound = position + scale * 0.5f; // 立方体的最大边界

    float tMin = (minBound.x - ray.origin.x) / ray.direction.x;
    float tMax = (maxBound.x - ray.origin.x) / ray.direction.x;

    if (tMin > tMax) std::swap(tMin, tMax);

    float tyMin = (minBound.y - ray.origin.y) / ray.direction.y;
    float tyMax = (maxBound.y - ray.origin.y) / ray.direction.y;

    if (tyMin > tyMax) std::swap(tyMin, tyMax);

    if ((tMin > tyMax) || (tyMin > tMax)) return false;

    tMin = std::max(tMin, tyMin);
    tMax = std::min(tMax, tyMax);

    float tzMin = (minBound.z - ray.origin.z) / ray.direction.z;
    float tzMax = (maxBound.z - ray.origin.z) / ray.direction.z;

    if (tzMin > tzMax) std::swap(tzMin, tzMax);

    if ((tMin > tzMax) || (tzMin > tMax)) return false;

    tMin = std::max(tMin, tzMin);
    tMax = std::min(tMax, tzMax);

    if (tMin < 0.001f || tMin >= isect.t) return false;

    // 成功命中
    isect.t = tMin;
    isect.position = ray.origin + tMin * ray.direction;
	// printf("position: %f, %f, %f\n", isect.position.x, isect.position.y, isect.position.z);
    glm::vec3 localPos = (isect.position - position) / scale; // 转换到局部坐标系
    glm::vec3 normal(0.0f);

	// 根据局部坐标确定法线方向
	// 1. 找出绝对值最大的分量，以确定光线击中了哪个面（X, Y, or Z）
	float absX = std::abs(localPos.x);
	float absY = std::abs(localPos.y);
	float absZ = std::abs(localPos.z);

	if (absX > absY && absX > absZ) {
		// 击中了 X 面 (左或右)
		// 通过 localPos.x 的符号判断是 +X 还是 -X 面
		normal = glm::vec3(localPos.x > 0 ? 1.0f : -1.0f, 0.0f, 0.0f);
	} else if (absY > absZ) {
		// 击中了 Y 面 (上或下)
		// 通过 localPos.y 的符号判断是 +Y 还是 -Y 面
		normal = glm::vec3(0.0f, localPos.y > 0 ? 1.0f : -1.0f, 0.0f);
	} else {
		// 击中了 Z 面 (前或后)
		// 通过 localPos.z 的符号判断是 +Z 还是 -Z 面
		normal = glm::vec3(0.0f, 0.0f, localPos.z > 0 ? 1.0f : -1.0f);
	}

	isect.normal = normal; // 归一化法线
	// isect.normal = glm::vec3(0, 1, 0);
    isect.material = getMaterial();
    isect.hit = true;

    return true;
}

void Cylinder::updateParameters(){
    float standardRatio = 2.0f; // 标准圆柱的比例: height = 2 * radius, radius = 1, height = 2
    float heightScale = height / (radius * standardRatio);
    scale = glm::vec3(radius, heightScale, radius); // 更新缩放比例
}

bool Cylinder::intersect(const Ray& ray, Intersection& isect) const {
    glm::vec3 oc = ray.origin - position;
    glm::vec3 dirXZ = glm::vec3(ray.direction.x, 0.0f, ray.direction.z);
    glm::vec3 ocXZ = glm::vec3(oc.x, 0.0f, oc.z);

    float a = glm::dot(dirXZ, dirXZ);
    float b = 2.0f * glm::dot(ocXZ, dirXZ);
    float c = glm::dot(ocXZ, ocXZ) - radius * radius;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0.0f) return false;

    float sqrtDisc = std::sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);

    float t = (t1 > 0.001f) ? t1 : ((t2 > 0.001f) ? t2 : -1.0f);
    if (t < 0.0f || t >= isect.t) return false;

    glm::vec3 hitPoint = ray.origin + t * ray.direction;
    if (hitPoint.y < position.y || hitPoint.y > position.y + height) return false; // 超出圆柱高度

    // 成功命中
    isect.t = t;
    isect.position = hitPoint;
    isect.normal = glm::normalize(glm::vec3(hitPoint.x - position.x, 0.0f, hitPoint.z - position.z));
    isect.material = getMaterial();
    isect.hit = true;

    return true;
}

bool Cone::intersect(const Ray& ray, Intersection& isect) const {
    glm::vec3 oc = ray.origin - position;
    float k = radius / height;
    float k2 = k * k;

    float a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z - k2 * ray.direction.y * ray.direction.y;
    float b = 2.0f * (oc.x * ray.direction.x + oc.z * ray.direction.z - k2 * oc.y * ray.direction.y);
    float c = oc.x * oc.x + oc.z * oc.z - k2 * oc.y * oc.y;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0.0f) return false;

    float sqrtDisc = std::sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);

    float t = (t1 > 0.001f) ? t1 : ((t2 > 0.001f) ? t2 : -1.0f);
    if (t < 0.0f || t >= isect.t) return false;

    glm::vec3 hitPoint = ray.origin + t * ray.direction;
    if (hitPoint.y < position.y || hitPoint.y > position.y + height) return false; // 超出圆锥高度

    // 成功命中
    isect.t = t;
    isect.position = hitPoint;
    isect.normal = glm::normalize(glm::vec3(hitPoint.x - position.x, -k * height, hitPoint.z - position.z));
    isect.material = getMaterial();
    isect.hit = true;

    return true;
}

bool Torus::intersect(const Ray& ray, Intersection& isect) const {
    // 环面交点计算较复杂，通常需要迭代求解
    // 这里提供一个简化的实现，假设环面位于XZ平面
    glm::vec3 oc = ray.origin - position;

    float major2 = majorRadius * majorRadius;
    float minor2 = minorRadius * minorRadius;

    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) + major2 - minor2;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0.0f) return false;

    float sqrtDisc = std::sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);

    float t = (t1 > 0.001f) ? t1 : ((t2 > 0.001f) ? t2 : -1.0f);
    if (t < 0.0f || t >= isect.t) return false;

    // 成功命中
    isect.t = t;
    isect.position = ray.origin + t * ray.direction;
    glm::vec3 hitPoint = isect.position - position;
    float distToCenter = glm::length(glm::vec3(hitPoint.x, 0.0f, hitPoint.z));
    isect.normal = glm::normalize(hitPoint - glm::vec3(position.x, position.y, position.z + distToCenter));
    isect.material = getMaterial();
    isect.hit = true;

    return true;
    // return mesh.intersect(ray, isect, material);
}