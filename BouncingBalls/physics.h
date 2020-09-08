#pragma once
#include <qtimer.h>
#include <qthread.h>
#include <vector>
#include <qdebug.h>
#include <qelapsedtimer.h>
#include <memory>
#include "geometry.h"
#include "windows.h"

class PhysicsEngine : public QThread {
	Q_OBJECT
		void run() override;

public:
	PhysicsEngine(Scene& scene, int fps = 300);
	~PhysicsEngine() { 
		terminate = true;
		// Hacky way to ensure loop exits by waiting a bit
		thread()->msleep(100); 
	}

	void flip() { running ^= 1; }

	void step() { stepping = true; }

	/* 
	   Waits for physics engine to finish current round before returning.
	   If processing takes too long it will return false, otherwise true.
	*/
	bool stop();

	void frame_tick() {
		#ifdef DEBUG
		qDebug() << "Physics FPS: " << frames;
		#endif
		frames = 0;
	}

	QTimer fpsTimer;
	QElapsedTimer deltaTimer;
	Scene& scene;
	int dt, fps, frames;
	bool running, stepping, terminate;
};
