#pragma once
#include <GL/glut.h>
#include <qdebug.h>
#include <vector>
#include <memory>
#include "constants.h"
#include "vector.h"
#include "force.h"

class Scene;

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
	float rad, m, r;
	Vec3f origPos;
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
