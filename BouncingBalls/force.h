#pragma once
#include "vector.h"

class Force {
public:
	Force(Vec3f dir, double force, double decayFactor = 0.2f)
		: dpc(dir), f(force), decayFactor(decayFactor)
	{
		if (decayFactor < 0) decayFactor = 0;
		dpc.normalize();
	}

	void decay() {
		if (f > 0) {
			f -= f * decayFactor;
			if (f < 0) f = 0;
		}
	}

	double f, decayFactor;
	Vec3f dpc;
};
