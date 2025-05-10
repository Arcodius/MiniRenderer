#include "Math.h"


bool Vec3::operator==(const Vec3& other) const {
    return std::abs(x - other.x) < EPSILON && std::abs(y - other.y) < EPSILON && std::abs(z - other.z) < EPSILON;
}
bool Vec3::operator!=(const Vec3& other) const {
    return !(*this == other);
}
Vec3& Vec3::operator+=(const Vec3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}
float Vec3::dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vec3 Vec3::cross(const Vec3& other) const {
    return Vec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

Vec3 Vec3::normalized() const {
    float len = length();
    return (len > 0) ? *this / len : Vec3(0, 0, 0);
}

Vec3 Vec3::clamp(const float MINVal, const float MAXVal) const {
    return Vec3(
        (std::max)(MINVal, (std::min)(MAXVal, x)),
        (std::max)(MINVal, (std::min)(MAXVal, y)),
        (std::max)(MINVal, (std::min)(MAXVal, z))
    );
}


Vec4& Vec4::operator+=(const Vec4& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

Vec4& Vec4::operator-=(const Vec4& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

Vec4& Vec4::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

Vec4& Vec4::operator/=(float scalar) {
    if (scalar == 0) {
        throw std::runtime_error("Division by zero");
    }
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

Vec4 Mat4::operator*(Vec4 vec) const {
    Vec4 result;

	result.x = data[0][0] * vec.x + data[0][1] * vec.y + data[0][2] * vec.z + data[0][3] * vec.w;
    result.y = data[1][0] * vec.x + data[1][1] * vec.y + data[1][2] * vec.z + data[1][3] * vec.w;
    result.z = data[2][0] * vec.x + data[2][1] * vec.y + data[2][2] * vec.z + data[2][3] * vec.w;
    result.w = data[3][0] * vec.x + data[3][1] * vec.y + data[3][2] * vec.z + data[3][3] * vec.w;

	return result;
}

Mat4 Mat4::operator*(const Mat4& other) const {
    Mat4 result;
    for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.data[i][j] = data[i][0] * other.data[0][j] +
								data[i][1] * other.data[1][j] +
								data[i][2] * other.data[2][j] +
								data[i][3] * other.data[3][j];
		}
	}
    return result;
}

std::ostream& operator<<(std::ostream& os, const Vec4& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

// Translate the matrix by given vector
Mat4& Mat4::translate(const Vec3& translation) {
    data[3][0] += translation.x; // 第3列第0行（tx）
    data[3][1] += translation.y; // 第3列第1行（ty）
    data[3][2] += translation.z; // 第3列第2行（tz）
    return *this;
}

// Rotate the matrix by given angles in radians
Mat4& Mat4::rotate(const Vec3& rotation){
    *this = rotateX(rotation.x) * rotateY(rotation.y) * rotateZ(rotation.z);
    return *this;
}

// 绕X轴旋转（角度为弧度）
Mat4& Mat4::rotateX(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    Mat4 rot(1.0f);
    rot[1][1] = c;  // 第1列第1行
    rot[2][1] = s;  // 第2列第1行
    rot[1][2] = -s; // 第1列第2行
    rot[2][2] = c;  // 第2列第2行
    *this = *this * rot;
    return *this;
}

// 绕Y轴旋转
Mat4& Mat4::rotateY(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    Mat4 rot(1.0f);
    rot[0][0] = c;  // 第0列第0行
    rot[2][0] = -s; // 第2列第0行
    rot[0][2] = s;  // 第0列第2行
    rot[2][2] = c;  // 第2列第2行
    *this = *this * rot;
    return *this;
}

// 绕Z轴旋转
Mat4& Mat4::rotateZ(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    Mat4 rot(1.0f);
    rot[0][0] = c;  // 第0列第0行
    rot[1][0] = s;  // 第1列第0行
    rot[0][1] = -s; // 第0列第1行
    rot[1][1] = c;  // 第1列第1行
    *this = *this * rot;
    return *this;
}

Mat4& Mat4::scale(const Vec3& scale) {
    Mat4 scaleMat(1.0f); // 初始化为单位矩阵
    scaleMat[0][0] = scale.x; // 第0列第0行（sx）
    scaleMat[1][1] = scale.y; // 第1列第1行（sy）
    scaleMat[2][2] = scale.z; // 第2列第2行（sz）
    *this = *this * scaleMat; // 右乘（列优先）
    return *this;
}

bool insideTriangle(const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c) {
	// 使用重心坐标法判断点p是否在三角形abc内
	Vec3 ab = b - a;
	Vec3 ac = c - a;
	Vec3 ap = p - a;

	float d00 = ab.dot(ab);
	float d01 = ab.dot(ac);
	float d11 = ac.dot(ac);
	float d20 = ap.dot(ab);
	float d21 = ap.dot(ac);

	float denom = d00 * d11 - d01 * d01;
	if (denom == 0) return false; // 三角形退化

	float v = (d11 * d20 - d01 * d21) / denom;
	if (v < 0 || v > 1) return false;

	float w = (d00 * d21 - d01 * d20) / denom;
	if (w < 0 || v + w > 1) return false;

	return true;
}