#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "physics.h"

class GLSimulation : public QOpenGLWidget, public QOpenGLFunctions {
	Q_OBJECT
public:
	explicit GLSimulation(QWidget* parent = 0);

	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void handleKeyobardEvents();
	Vec3f mouseToWorld(int x, int y);

	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;

	void wheelEvent(QWheelEvent* event) override;

	void closeEvent(QCloseEvent* event) override;

	void frame_tick();
	void renderLoop();
	void generateBalls();

signals:
	void massChanged(double mass);
	void restitutionChanged(double res);
	void radiusChanged(double radius);
	void xChanged(int x);
	void yChanged(int y);
	void zChanged(int z);
	void vxChanged(double v);
	void vyChanged(double v);
	void vzChanged(double v);
	
public slots:
	void updateMass(double mass);
	void updateRestitution(double res);
	void updateRadius(double radius);
	void updateX(int x);
	void updateY(int y);
	void updateZ(int z);
	void updateVx(double v);
	void updateVy(double v);
	void updateVz(double v);
	void updateCameraSpeed(double v);
	void updateFPS(int fps);
	void updatePhysicsFPS(int physFPS);
	void animationButtonPressed();
	void stepButtonPressed();
	void resetCurrentButtonPressed();
	void resetAllButtonPressed();
	void clearAllButtonPressed();
	void addExternalForce(Vec3f& dir, float power, float decay);
	void switchWallsButtonPressed(bool state);

private:
	int fps, frames;
	float lastX, lastY;
	float zoom, fov;
	Camera3D camera;
	QMap<int, bool> keystates;
	Scene world;
	QTimer fpsTimer;

	Sphere* selected;
	PhysicsEngine* physEngine;
};
