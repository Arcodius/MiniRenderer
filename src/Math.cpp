#include "Math.h"

// Vec2 implementations
Vec2::Vec2() : x(0.0f), y(0.0f) {}
Vec2::Vec2(float x, float y) : x(x), y(y) {}

float Vec2::length() const {
    return std::sqrt(x * x + y * y);
}

Vec2 Vec2::normalized() const {
    float len = length();
    if (len < EPSILON) return Vec2(0.0f, 0.0f);
    return Vec2(x / len, y / len);
}

float Vec2::dot(const Vec2& other) const {
    return x * other.x + y * other.y;
}

Vec2 Vec2::operator+(const Vec2& other) const {
    return Vec2(x + other.x, y + other.y);
}

Vec2 Vec2::operator-(const Vec2& other) const {
    return Vec2(x - other.x, y - other.y);
}

Vec2 Vec2::operator*(float scalar) const {
    return Vec2(x * scalar, y * scalar);
}

Vec2 Vec2::operator/(float scalar) const {
    if (std::abs(scalar) < EPSILON) throw std::runtime_error("Division by zero");
    return Vec2(x / scalar, y / scalar);
}

Vec2& Vec2::operator+=(const Vec2& other) {
    x += other.x;
    y += other.y;
    return *this;
}

Vec2& Vec2::operator-=(const Vec2& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

Vec2& Vec2::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

Vec2& Vec2::operator/=(float scalar) {
    if (std::abs(scalar) < EPSILON) throw std::runtime_error("Division by zero");
    x /= scalar;
    y /= scalar;
    return *this;
}

bool Vec2::operator==(const Vec2& other) const {
    return std::abs(x - other.x) < EPSILON && std::abs(y - other.y) < EPSILON;
}

bool Vec2::operator!=(const Vec2& other) const {
    return !(*this == other);
}

// Vec3 implementations
Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
Vec3::Vec3(float x) : x(x), y(x), z(x) {}
Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

float Vec3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::normalized() const {
    float len = length();
    if (len < EPSILON) return Vec3(0.0f, 0.0f, 0.0f);
    return Vec3(x / len, y / len, z / len);
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

Vec3 Vec3::operator+(const Vec3& other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
}

Vec3 Vec3::operator-(const Vec3& other) const {
    return Vec3(x - other.x, y - other.y, z - other.z);
}

Vec3 Vec3::operator*(float scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
}
Vec3 operator*(float scalar, const Vec3& vec) {
    return Vec3(scalar * vec.x, scalar * vec.y, scalar * vec.z);
}

Vec3 Vec3::operator/(float scalar) const {
    if (std::abs(scalar) < EPSILON) throw std::runtime_error("Division by zero");
    return Vec3(x / scalar, y / scalar, z / scalar);
}

Vec3& Vec3::operator+=(const Vec3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vec3& Vec3::operator-=(const Vec3& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vec3& Vec3::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vec3& Vec3::operator/=(float scalar) {
    if (std::abs(scalar) < EPSILON) throw std::runtime_error("Division by zero");
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

bool Vec3::operator==(const Vec3& other) const {
    return std::abs(x - other.x) < EPSILON && 
           std::abs(y - other.y) < EPSILON && 
           std::abs(z - other.z) < EPSILON;
}

bool Vec3::operator!=(const Vec3& other) const {
    return !(*this == other);
}

Vec3 Vec3::clamp(float minVal, float maxVal) const {
    return Vec3(
        CLAMP(x, minVal, maxVal),
        CLAMP(y, minVal, maxVal),
        CLAMP(z, minVal, maxVal)
    );
}

// Vec4 implementations
Vec4::Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
Vec4::Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
Vec4::Vec4(const Vec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}

float Vec4::length() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

Vec4 Vec4::normalized() const {
    float len = length();
    if (len < EPSILON) return Vec4(0.0f, 0.0f, 0.0f, 0.0f);
    return Vec4(x / len, y / len, z / len, w / len);
}

float Vec4::dot(const Vec4& other) const {
    return x * other.x + y * other.y + z * other.z + w * other.w;
}

Vec4 Vec4::operator+(const Vec4& other) const {
    return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
}

Vec4 Vec4::operator-(const Vec4& other) const {
    return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
}

Vec4 Vec4::operator*(float scalar) const {
    return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
}

Vec4 Vec4::operator/(float scalar) const {
    if (std::abs(scalar) < EPSILON) throw std::runtime_error("Division by zero");
    return Vec4(x / scalar, y / scalar, z / scalar, w / scalar);
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
    if (std::abs(scalar) < EPSILON) throw std::runtime_error("Division by zero");
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

float& Vec4::operator[](int index){
    if (index < 0 || index >= 4) throw std::out_of_range("Index out of range");
    switch (index) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        case 3: return w;
        default: throw std::out_of_range("Index out of range");
    }
}
float Vec4::operator[](int index) const{
    if (index < 0 || index >= 4) throw std::out_of_range("Index out of range");
    switch (index) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        case 3: return w;
        default: throw std::out_of_range("Index out of range");
    }
}

Vec3 Vec4::toVec3() const {
    return Vec3(x, y, z);
}

// Mat4 implementations
Mat4::Mat4() {
    data.fill(0.0f);
}

Mat4 Mat4::identity() {
    Mat4 result;
    result(0, 0) = 1.0f;
    result(1, 1) = 1.0f;
    result(2, 2) = 1.0f;
    result(3, 3) = 1.0f;
    return result;
}

float& Mat4::operator()(int row, int col) {
    return data[row * 4 + col];
}

float Mat4::operator()(int row, int col) const {
    return data[row * 4 + col];
}

Mat4 Mat4::operator*(const Mat4& other) const {
    Mat4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                result(i, j) += (*this)(i, k) * other(k, j);
            }
        }
    }
    return result;
}

Vec4 Mat4::operator*(const Vec4& vec) const {
    return Vec4(
        (*this)(0, 0) * vec.x + (*this)(0, 1) * vec.y + (*this)(0, 2) * vec.z + (*this)(0, 3) * vec.w,
        (*this)(1, 0) * vec.x + (*this)(1, 1) * vec.y + (*this)(1, 2) * vec.z + (*this)(1, 3) * vec.w,
        (*this)(2, 0) * vec.x + (*this)(2, 1) * vec.y + (*this)(2, 2) * vec.z + (*this)(2, 3) * vec.w,
        (*this)(3, 0) * vec.x + (*this)(3, 1) * vec.y + (*this)(3, 2) * vec.z + (*this)(3, 3) * vec.w
    );
}

// Row-major vector * matrix multiplication
Vec4 operator*(const Vec4& vec, const Mat4& mat) {
    return Vec4(
        vec.x * mat(0, 0) + vec.y * mat(1, 0) + vec.z * mat(2, 0) + vec.w * mat(3, 0),
        vec.x * mat(0, 1) + vec.y * mat(1, 1) + vec.z * mat(2, 1) + vec.w * mat(3, 1),
        vec.x * mat(0, 2) + vec.y * mat(1, 2) + vec.z * mat(2, 2) + vec.w * mat(3, 2),
        vec.x * mat(0, 3) + vec.y * mat(1, 3) + vec.z * mat(2, 3) + vec.w * mat(3, 3)
    );
}

Mat4 Mat4::transposed() const {
    Mat4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result(j, i) = (*this)(i, j);
        }
    }
    return result;
}

Mat4 Mat4::inverted() const {
    Mat4 inv;
    float det;
    
    inv(0, 0) = data[5] * data[10] * data[15] - data[5] * data[11] * data[14] - 
                data[9] * data[6] * data[15] + data[9] * data[7] * data[14] + 
                data[13] * data[6] * data[11] - data[13] * data[7] * data[10];
    inv(1, 0) = -data[4] * data[10] * data[15] + data[4] * data[11] * data[14] + 
                data[8] * data[6] * data[15] - data[8] * data[7] * data[14] - 
                data[12] * data[6] * data[11] + data[12] * data[7] * data[10];
    inv(2, 0) = data[4] * data[9] * data[15] - data[4] * data[11] * data[13] - 
                data[8] * data[5] * data[15] + data[8] * data[7] * data[13] + 
                data[12] * data[5] * data[11] - data[12] * data[7] * data[9];
    inv(3, 0) = -data[4] * data[9] * data[14] + data[4] * data[10] * data[13] + 
                data[8] * data[5] * data[14] - data[8] * data[6] * data[13] - 
                data[12] * data[5] * data[10] + data[12] * data[6] * data[9];
    inv(0, 1) = -data[1] * data[10] * data[15] + data[1] * data[11] * data[14] + 
                data[9] * data[2] * data[15] - data[9] * data[3] * data[14] - 
                data[13] * data[2] * data[11] + data[13] * data[3] * data[10];
    inv(1, 1) = data[0] * data[10] * data[15] - data[0] * data[11] * data[14] - 
                data[8] * data[2] * data[15] + data[8] * data[3] * data[14] + 
                data[12] * data[2] * data[11] - data[12] * data[3] * data[10];
    inv(2, 1) = -data[0] * data[9] * data[15] + data[0] * data[11] * data[13] + 
                data[8] * data[1] * data[15] - data[8] * data[3] * data[13] - 
                data[12] * data[1] * data[11] + data[12] * data[3] * data[9];
    inv(3, 1) = data[0] * data[9] * data[14] - data[0] * data[10] * data[13] - 
                data[8] * data[1] * data[14] + data[8] * data[2] * data[13] + 
                data[12] * data[1] * data[10] - data[12] * data[2] * data[9];
    inv(0, 2) = data[1] * data[6] * data[15] - data[1] * data[7] * data[14] - 
                data[5] * data[2] * data[15] + data[5] * data[3] * data[14] + 
                data[13] * data[2] * data[7] - data[13] * data[3] * data[6];
    inv(1, 2) = -data[0] * data[6] * data[15] + data[0] * data[7] * data[14] + 
                data[4] * data[2] * data[15] - data[4] * data[3] * data[14] - 
                data[12] * data[2] * data[7] + data[12] * data[3] * data[6];
    inv(2, 2) = data[0] * data[5] * data[15] - data[0] * data[7] * data[13] - 
                data[4] * data[1] * data[15] + data[4] * data[3] * data[13] + 
                data[12] * data[1] * data[7] - data[12] * data[3] * data[5];
    inv(3, 2) = -data[0] * data[5] * data[14] + data[0] * data[6] * data[13] + 
                data[4] * data[1] * data[14] - data[4] * data[2] * data[13] - 
                data[12] * data[1] * data[6] + data[12] * data[2] * data[5];
    inv(0, 3) = -data[1] * data[6] * data[11] + data[1] * data[7] * data[10] + 
                data[5] * data[2] * data[11] - data[5] * data[3] * data[10] - 
                data[9] * data[2] * data[7] + data[9] * data[3] * data[6];
    inv(1, 3) = data[0] * data[6] * data[11] - data[0] * data[7] * data[10] - 
                data[4] * data[2] * data[11] + data[4] * data[3] * data[10] + 
                data[8] * data[2] * data[7] - data[8] * data[3] * data[6];
    inv(2, 3) = -data[0] * data[5] * data[11] + data[0] * data[7] * data[9] + 
                data[4] * data[1] * data[11] - data[4] * data[3] * data[9] - 
                data[8] * data[1] * data[7] + data[8] * data[3] * data[5];
    inv(3, 3) = data[0] * data[5] * data[10] - data[0] * data[6] * data[9] - 
                data[4] * data[1] * data[10] + data[4] * data[2] * data[9] + 
                data[8] * data[1] * data[6] - data[8] * data[2] * data[5];

    det = data[0] * inv(0, 0) + data[1] * inv(1, 0) + data[2] * inv(2, 0) + data[3] * inv(3, 0);

    if (std::abs(det) < EPSILON) {
        throw std::runtime_error("Matrix is not invertible");
    }

    det = 1.0f / det;

    for (int i = 0; i < 16; i++) {
        inv.data[i] *= det;
    }

    return inv;
}

Mat4 Mat4::translation(const Vec3& t) {
    Mat4 result = identity();
    result(0, 3) = t.x;
    result(1, 3) = t.y;
    result(2, 3) = t.z;
    return result;
}

Mat4 Mat4::scale(const Vec3& s) {
    Mat4 result = identity();
    result(0, 0) = s.x;
    result(1, 1) = s.y;
    result(2, 2) = s.z;
    return result;
}

Mat4 Mat4::rotationX(float angleRad) {
    Mat4 result = identity();
    float c = std::cos(angleRad);
    float s = std::sin(angleRad);
    result(1, 1) = c;
    result(1, 2) = -s;
    result(2, 1) = s;
    result(2, 2) = c;
    return result;
}

Mat4 Mat4::rotationY(float angleRad) {
    Mat4 result = identity();
    float c = std::cos(angleRad);
    float s = std::sin(angleRad);
    result(0, 0) = c;
    result(0, 2) = s;
    result(2, 0) = -s;
    result(2, 2) = c;
    return result;
}

Mat4 Mat4::rotationZ(float angleRad) {
    Mat4 result = identity();
    float c = std::cos(angleRad);
    float s = std::sin(angleRad);
    result(0, 0) = c;
    result(0, 1) = -s;
    result(1, 0) = s;
    result(1, 1) = c;
    return result;
}

Mat4 Mat4::perspective(float fovYRad, float aspect, float nearZ, float farZ) {
    Mat4 result;
    float tanHalfFov = std::tan(fovYRad * 0.5f);
    
    result(0, 0) = 1.0f / (aspect * tanHalfFov);
    result(1, 1) = 1.0f / tanHalfFov;
    result(2, 2) = -(farZ + nearZ) / (farZ - nearZ);
    result(2, 3) = -(2.0f * farZ * nearZ) / (farZ - nearZ);
    result(3, 2) = -1.0f;
    
    return result;
}

Mat4 Mat4::orthographic(float left, float right, float bottom, float top, float nearZ, float farZ) {
    Mat4 result = identity();
    
    result(0, 0) = 2.0f / (right - left);
    result(1, 1) = 2.0f / (top - bottom);
    result(2, 2) = -2.0f / (farZ - nearZ);
    result(0, 3) = -(right + left) / (right - left);
    result(1, 3) = -(top + bottom) / (top - bottom);
    result(2, 3) = -(farZ + nearZ) / (farZ - nearZ);
    
    return result;
}

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = (center - eye).normalized();
    Vec3 u = up.normalized();
    Vec3 s = f.cross(u).normalized();
    u = s.cross(f);

    Mat4 result = identity();
    result(0, 0) = s.x;
    result(0, 1) = s.y;
    result(0, 2) = s.z;
    result(1, 0) = u.x;
    result(1, 1) = u.y;
    result(1, 2) = u.z;
    result(2, 0) = -f.x;
    result(2, 1) = -f.y;
    result(2, 2) = -f.z;
    result(0, 3) = -s.dot(eye);
    result(1, 3) = -u.dot(eye);
    result(2, 3) = f.dot(eye);
    
    return result;
}

// Geometric functions
bool insideTriangle(const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c) {
    Vec3 v0 = c - a;
    Vec3 v1 = b - a;
    Vec3 v2 = p - a;

    float dot00 = v0.dot(v0);
    float dot01 = v0.dot(v1);
    float dot02 = v0.dot(v2);
    float dot11 = v1.dot(v1);
    float dot12 = v1.dot(v2);

    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

float barycentricArea(const Vec3& a, const Vec3& b, const Vec3& c) {
    return 0.5f * (b - a).cross(c - a).length();
}

// Stream operators
std::ostream& operator<<(std::ostream& os, const Vec2& v) {
    return os << "(" << v.x << ", " << v.y << ")";
}

std::ostream& operator<<(std::ostream& os, const Vec3& v) {
    return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

std::ostream& operator<<(std::ostream& os, const Vec4& v) {
    return os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}

std::ostream& operator<<(std::ostream& os, const Mat4& m) {
    os << "[\n";
    for (int i = 0; i < 4; ++i) {
        os << "  [" << m(i, 0) << ", " << m(i, 1) << ", " << m(i, 2) << ", " << m(i, 3) << "]\n";
    }
    os << "]";
    return os;
}

