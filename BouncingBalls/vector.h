#pragma once
#include <qdebug.h>

class Vec3f {
public:
	Vec3f() : x(0), y(0), z(0) {}
	Vec3f(const Vec3f& other) : x(other.x), y(other.y), z(other.z) {}
	Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}

	float normsq() { return x * x + y * y + z * z; }
	float norm() { return sqrtf(normsq()); }
	float dot(const Vec3f& other) { return x * other.x + y * other.y + z * other.z; }

	void normalize() {
		float l = norm();
		if (l > 0)
			x /= l, y /= l, z /= l;
	}

	Vec3f cross(const Vec3f& other) {
		return Vec3f(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
	}

	operator float* () { return reinterpret_cast<float*>(this); }

	Vec3f& operator=(const Vec3f& right) {
		x = right.x, y = right.y, z = right.z;
		return *this;
	}

	Vec3f& operator+=(const Vec3f& right) {
		x += right.x, y += right.y, z += right.z;
		return *this;
	}

	Vec3f& operator-=(const Vec3f& right) {
		x -= right.x, y -= right.y, z -= right.z;
		return *this;
	}

	float x, y, z;

	friend Vec3f operator-(const Vec3f& v1, const Vec3f& v2) {
		return Vec3f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
	}

	friend Vec3f operator+(const Vec3f& v1, const Vec3f& v2) {
		return Vec3f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
	}

	friend Vec3f operator*(float scalar, Vec3f const& vec) {
		return Vec3f(vec.x * scalar, vec.y * scalar, vec.z * scalar);
	}

	friend Vec3f operator-(const Vec3f& vec) {
		return Vec3f(-vec.x, -vec.y, -vec.z);
	}

	friend Vec3f operator/(Vec3f const& vec, float scalar) {
		return Vec3f(vec.x / scalar, vec.y / scalar, vec.z / scalar);
	}

	friend QDebug operator<<(QDebug out, Vec3f const& vec) {
		out << "(" << vec.x << "," << vec.y << "," << vec.z << ")";
		return out;
	}
};
