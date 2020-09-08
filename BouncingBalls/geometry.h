#pragma once
#include <GL/glut.h>
#include <qdebug.h>
#include <vector>
#include <memory>
#include "constants.h"

class Scene;

class Vec3f {
	public:
		Vec3f() : x(0), y(0), z(0) {}
		Vec3f(const Vec3f& other) : x(other.x), y(other.y), z(other.z) {}
		Vec3f(float x, float y, float z): x(x), y(y), z(z) {}

		float normsq() { return x * x + y * y + z * z; }
		float norm() { return sqrtf(normsq()); }
		float dot(const Vec3f& other) { return x * other.x + y * other.y + z * other.z; }

		void normalize();

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

class Force {
public:
	Force(Vec3f dir, double force, double decayFactor = 0.2f);

	void decay();

	double f, decayFactor;
	Vec3f dpc;
};

class Camera3D {
public:
	Camera3D(float velocity = 0.3): Camera3D(0, 0, 0, velocity) {}
	Camera3D(float x, float y, float z, float velocity = 0.3) : x(x), y(y), z(z), rotX(0), rotY(0), v(velocity) {}
	Camera3D(Vec3f& xyz, float velocity = 0.3): Camera3D(xyz.x, xyz.y, xyz.z, velocity) {}

	float x, y, z;
	float rotX, rotY;
	float v;
};

class GeomObject {
public:
	virtual void draw() {}
	virtual void reset() {}
	virtual void update(double elapsedTime) {}
	virtual void collide(Scene& objects, int idx) {}
};

class Sphere : public GeomObject {
public:
	Sphere(Vec3f& position, float radius, float mass, float restitution = 0.8f,
		   Vec3f& velocity = Vec3f(0, 0, 0), Vec3f& color = Vec3f(1, 0.9, 0.9),
		   Vec3f& selectedColor = Vec3f(0.9, 0.1, 0.1));

	void draw() override;
	void reset() override;
	void update(double dt) override;
	void collide(Scene& scene, int idx) override;

	Vec3f pos;
	//float x, y, z;
	float rad, m, r;
	Vec3f origPos;
	//float origX, origY, origZ;
	Vec3f rgb, selectRgb;
	bool selected;
	std::vector<Force> forces;
	Vec3f velocity;
	Vec3f origVelocity;
};

class Plane : public GeomObject {
public:
	Plane(Vec3f& a, Vec3f& b, Vec3f& c, Vec3f& d, Vec3f& color);

	void draw() override;

	Vec3f a, b, c, d;
	Vec3f rgb;
	Vec3f normal;
};

class AABB : public Plane {
public:
	AABB(Vec3f& a, Vec3f& b, Vec3f& c, Vec3f& d, Vec3f& color);

	float minX, minY, minZ, maxX, maxY, maxZ;
};

// Use a master class with all possible types of geometry, instead of polymorphism, 
// to avoid dynamic casting for collision detection.
class Scene {
public:
	Scene() {}

	std::vector<std::unique_ptr<Sphere>> spheres;
	std::vector<std::unique_ptr<Plane>> planes;
	std::vector<std::unique_ptr<AABB>> aabbs;
};
