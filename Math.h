#pragma once
#include <array>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}

    float length() const {
		return std::sqrt(x * x + y * y);
	}

    Vec2 operator+(const Vec2& other) const {
		return Vec2(x + other.x, y + other.y);
	}

    Vec2 operator-(const Vec2& other) const {
		return Vec2(x - other.x, y - other.y);
	}

    Vec2 operator*(float scalar) const {
		return Vec2(x * scalar, y * scalar);
	}
    friend Vec2 operator*(float scalar, const Vec2& vec) {
        return vec * scalar;
    }

    Vec2 operator/(float scalar) const {
		if (scalar == 0) {
			throw std::runtime_error("Division by zero");
		}
		return Vec2(x / scalar, y / scalar);
	}

	float dot(const Vec2& other) const {
		return x * other.x + y * other.y;
	}

	Vec2 normalized() const {
		float len = length();
		return (len > 0) ? *this / len : Vec2(0, 0);
	}
};

struct Vec3 {
	float x, y, z;
	Vec3() : x(0), y(0), z(0) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

	float length() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	Vec3 operator+(const Vec3& other) const {
		return Vec3(x + other.x, y + other.y, z + other.z);
	}

	Vec3 operator-(const Vec3& other) const {
		return Vec3(x - other.x, y - other.y, z - other.z);
	}

	Vec3 operator*(float scalar) const {
		return Vec3(x * scalar, y * scalar, z * scalar);
	};
	friend Vec3 operator*(float scalar, const Vec3& vec) {
		return vec * scalar;
	}

	Vec3 operator/(float scalar) const {
		if (scalar == 0) {
			throw std::runtime_error("Division by zero");
		}
		return Vec3(x / scalar, y / scalar, z / scalar);
	}

	float dot(const Vec3& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	Vec3 cross(const Vec3& other) const {
		return Vec3(
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x
		);
	}

	Vec3 normalized() const {
		float len = length();
		return (len > 0) ? *this / len : Vec3(0, 0, 0);
	}

    friend std::ostream& operator<<(std::ostream& os, const Vec3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
};

struct Vec4 {
    float x, y, z, w;

    // Constructors
    Vec4() : x(0), y(0), z(0), w(1) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec3& vec3, float w = 1.0f) : x(vec3.x), y(vec3.y), z(vec3.z), w(w) {}

    // Length operations
    float length() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    float lengthSquared() const {
        return x * x + y * y + z * z + w * w;
    }

    // Arithmetic operators
    Vec4 operator+(const Vec4& other) const {
        return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    Vec4 operator-(const Vec4& other) const {
        return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    Vec4 operator*(float scalar) const {
        return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    friend Vec4 operator*(float scalar, const Vec4& vec) {
        return vec * scalar;
    }

    Vec4 operator/(float scalar) const {
        if (scalar == 0) {
            throw std::runtime_error("Division by zero");
        }
        return Vec4(x / scalar, y / scalar, z / scalar, w / scalar);
    }

    Vec4& operator+=(const Vec4& other);
    Vec4& operator-=(const Vec4& other);
    Vec4& operator*=(float scalar);
    Vec4& operator/=(float scalar);
    friend std::ostream& operator<<(std::ostream& os, const Vec4& v);

    // Dot product
    float dot(const Vec4& other) const {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    // Normalization
    Vec4 normalized() const {
        float len = length();
        return (len > 0) ? *this / len : Vec4(0, 0, 0, 1);
    }

    // Homogeneous division (for converting from homogeneous coordinates)
    Vec3 homogeneousDivide() const {
        if (w != 0) {
            return Vec3(x / w, y / w, z / w);
        }
        return Vec3(x, y, z);
    }

    // Component-wise multiplication
    Vec4 componentMul(const Vec4& other) const {
        return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    // Conversion to Vec3 (drops w component)
    Vec3 xyz() const {
        return Vec3(x, y, z);
    }
};

struct Mat4 {
    // Data storage - 4x4 matrix stored in column-major order
    std::array<std::array<float, 4>, 4> data;

    // Constructors
    Mat4() { identity(); }

    Mat4(float diagonal) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                data[i][j] = (i == j) ? diagonal : 0.0f;
            }
        }
    }

    // Identity matrix
    void identity() {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                data[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
    }
    
    static Mat4 zero() {
		Mat4 result;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				result.data[i][j] = 0.0f;
			}
		}
		return result;
	}

    // Matrix multiplication
    Mat4 operator*(const Mat4& other) const;
    Vec4 operator*(Vec4 vec) const;

    // Access operators
    float* operator[](int index) { return data[index].data(); }
    const float* operator[](int index) const { return data[index].data(); }

    // Scalar multiplication
    Mat4 operator*(float scalar) const {
        Mat4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.data[i][j] = data[i][j] * scalar;
            }
        }
        return result;
    }

    // Matrix addition
    Mat4 operator+(const Mat4& other) const {
        Mat4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.data[i][j] = data[i][j] + other.data[i][j];
            }
        }
        return result;
    }

    // Matrix subtraction
    Mat4 operator-(const Mat4& other) const {
        Mat4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.data[i][j] = data[i][j] - other.data[i][j];
            }
        }
        return result;
    }

    Mat4& translate(const Vec3& translation);
    Mat4& rotate(const Vec3& rotation);
    Mat4& rotateX(float radian);
    Mat4& rotateY(float radian);
    Mat4& rotateZ(float radian);
    Mat4& scale(const Vec3& scale);

    // Transpose
    Mat4 transpose() const {
        Mat4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.data[i][j] = data[j][i];
            }
        }
        return result;
    }

    // Print matrix (for debugging)
    void print() const {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                std::cout << data[j][i] << " ";
            }
            std::cout << std::endl;
        }
    }
};

// Non-member scalar multiplication (to support float * Mat4)
inline Mat4 operator*(float scalar, const Mat4& mat) {
    return mat * scalar;
}