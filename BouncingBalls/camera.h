#pragma once
#include "vector.h"

class Camera3D {
public:
	Camera3D(float velocity = 0.3) : Camera3D(0, 0, 0, velocity) {}
	Camera3D(float x, float y, float z, float velocity = 0.3) : x(x), y(y), z(z), rotX(0), rotY(0), v(velocity) {}
	Camera3D(Vec3f& xyz, float velocity = 0.3) : Camera3D(xyz.x, xyz.y, xyz.z, velocity) {}

	float x, y, z;
	float rotX, rotY;
	float v;
};
