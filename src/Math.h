#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <sstream>

#define PI 3.14159265358979323846f
#define EPSILON 1e-6f
#define RADIAN(degree) ((degree) * PI / 180.0f)
#define DEGREE(radian) ((radian) * 180.0f / PI)
#define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))
#define FLT_MAX std::numeric_limits<float>::max()
#define FLT_MIN std::numeric_limits<float>::min()
#define SWAP(a, b) { auto temp = a; a = b; b = temp; }

class Vec2 {
public:
    float x, y;

    Vec2();
    Vec2(float x, float y);

    float length() const;
    Vec2 normalized() const;

    float dot(const Vec2& other) const;

    Vec2 operator+(const Vec2& other) const;
    Vec2 operator-(const Vec2& other) const;
    Vec2 operator*(float scalar) const;
    Vec2 operator/(float scalar) const;

    Vec2& operator+=(const Vec2& other);
    Vec2& operator-=(const Vec2& other);
    Vec2& operator*=(float scalar);
    Vec2& operator/=(float scalar);

    bool operator==(const Vec2& other) const;
    bool operator!=(const Vec2& other) const;
};

class Vec3 {
public:
    float x, y, z;

    Vec3();
    Vec3(float x);
    Vec3(float x, float y, float z);

    float length() const;
    Vec3 normalized() const;

    float dot(const Vec3& other) const;
    Vec3 cross(const Vec3& other) const;

    Vec3 operator+(const Vec3& other) const;
    Vec3 operator-(const Vec3& other) const;
    Vec3 operator*(float scalar) const;
    friend Vec3 operator*(float scalar, const Vec3& vec);
    Vec3 operator/(float scalar) const;

    Vec3& operator+=(const Vec3& other);
    Vec3& operator-=(const Vec3& other);
    Vec3& operator*=(float scalar);
    Vec3& operator/=(float scalar);

    bool operator==(const Vec3& other) const;
    bool operator!=(const Vec3& other) const;

    Vec3 clamp(float minVal, float maxVal) const;
};

class Vec4 {
public:
    float x, y, z, w;

    Vec4();
    Vec4(float x, float y, float z, float w);
    Vec4(const Vec3& v, float w = 1.0f);

    float length() const;
    Vec4 normalized() const;

    float dot(const Vec4& other) const;

    Vec4 operator+(const Vec4& other) const;
    Vec4 operator-(const Vec4& other) const;
    Vec4 operator*(float scalar) const;
    Vec4 operator/(float scalar) const;

    Vec4& operator+=(const Vec4& other);
    Vec4& operator-=(const Vec4& other);
    Vec4& operator*=(float scalar);
    Vec4& operator/=(float scalar);

    float& operator[](int index);
    float operator[](int index) const;

    Vec3 toVec3() const;
};

// Proxy class for matrix row access
class Mat4Row {
private:
    float* row_data;
    
public:
    Mat4Row(float* data) : row_data(data) {}
    
    float& operator[](int col) {
        if (col < 0 || col >= 4) {
            throw std::out_of_range("Matrix column index out of range");
        }
        return row_data[col];
    }
    
    const float& operator[](int col) const {
        if (col < 0 || col >= 4) {
            throw std::out_of_range("Matrix column index out of range");
        }
        return row_data[col];
    }
};
// Const version of the proxy class
class Mat4ConstRow {
private:
    const float* row_data;
    
public:
    Mat4ConstRow(const float* data) : row_data(data) {}
    
    const float& operator[](int col) const {
        if (col < 0 || col >= 4) {
            throw std::out_of_range("Matrix column index out of range");
        }
        return row_data[col];
    }
};

// 4x4 Matrix, Row-Major
class Mat4 {
public:
    std::array<float, 16> data;

    Mat4();
    static Mat4 identity();

    float& operator()(int row, int col);
    float operator()(int row, int col) const;
    // Add subscript operators for matrix[i][j] access
    Mat4Row operator[](int row) {
        if (row < 0 || row >= 4) {
            throw std::out_of_range("Matrix row index out of range");
        }
        return Mat4Row(&data[row * 4]);
    }
    
    Mat4ConstRow operator[](int row) const {
        if (row < 0 || row >= 4) {
            throw std::out_of_range("Matrix row index out of range");
        }
        return Mat4ConstRow(&data[row * 4]);
    }

    Mat4 operator*(const Mat4& other) const;
    Vec4 operator*(const Vec4& vec) const;
    friend Vec4 operator*(const Vec4& vec, const Mat4& mat); // Row-major

    Mat4 transposed() const;
    Mat4 inverted() const;

    static Mat4 translation(const Vec3& t);
    static Mat4 scale(const Vec3& s);
    static Mat4 rotationX(float angleRad);
    static Mat4 rotationY(float angleRad);
    static Mat4 rotationZ(float angleRad);
    static Mat4 perspective(float fovYRad, float aspect, float nearZ, float farZ);
    static Mat4 orthographic(float left, float right, float bottom, float top, float nearZ, float farZ);
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
};

// 常用几何判断函数
bool insideTriangle(const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c);
float barycentricArea(const Vec3& a, const Vec3& b, const Vec3& c);

std::ostream& operator<<(std::ostream& os, const Vec2& v);
std::ostream& operator<<(std::ostream& os, const Vec3& v);
std::ostream& operator<<(std::ostream& os, const Vec4& v);
std::ostream& operator<<(std::ostream& os, const Mat4& m);