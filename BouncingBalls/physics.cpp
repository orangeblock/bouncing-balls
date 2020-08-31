#include "physics.h"

PhysicsEngine::PhysicsEngine(Scene& scene, int fps)
	: scene(scene), running(false), stepping(false), terminate(false), dt(0), frames(0), fps(fps)
{
	fpsTimer.setTimerType(Qt::PreciseTimer);
	connect(&fpsTimer, &QTimer::timeout, this, &PhysicsEngine::frame_tick);
	fpsTimer.start(1000);
	deltaTimer.start();
}

bool PhysicsEngine::stop(){
	// Enable step mode so we know when physics loop exits
	running = false;
	stepping = true;
	for (int i = 0; i < 10; ++i) {
		thread()->msleep(10);
		if (!stepping) { return true; }
	}
	return false;
}

void PhysicsEngine::run(){
	while (!terminate) {
		if (running || stepping) {
			double elapsedSec;
			if (stepping) elapsedSec = 1.0f / fps;
			else elapsedSec = dt / 1000.0f;

			int nspheres = scene.spheres.size();
			for (int i = 0; i < nspheres; i++) {
				if (scene.spheres[i] != nullptr) {
					scene.spheres[i]->update(elapsedSec);
					scene.spheres[i]->collide(scene, i);
				}
			}
			if (stepping) stepping ^= 1;
		}
		dt = deltaTimer.restart();
		int mpf = 1000.0f / fps;
		if (dt > 500) dt = mpf;
		int rem = dt % mpf;
		thread()->msleep(mpf - rem);
		frames++;
	}
}
