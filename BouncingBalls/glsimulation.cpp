#include "glsimulation.h"
#include <QKeyEvent>
#include <qtimer.h>
#include <qtime>
#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <qthread.h>
#include <qdebug.h>
#include <vector>
#include "geometry.h"
#include <math.h>
#include <memory>
#include "constants.h"

GLSimulation::GLSimulation(QWidget* parent)
	: QOpenGLWidget(parent), fps(60), camera(Camera3D(0, 10, 1)), selected(nullptr),
	zoom(1.0f), fov(45.0f), frames(0)
{
	// Setup scene
	world.planes.push_back(std::make_unique<Plane>(Plane(Vec3f(-30, 0, 30), Vec3f(30, 0, 30), Vec3f(30, 0, -30), Vec3f(-30, 0, -30), Vec3f(0.5, 0.7, 0.5))));
	world.spheres.push_back(std::make_unique<Sphere>(Sphere(Vec3f(0, 10, 0), 0.2, 1)));

	// Start the physics engine in a separate thread
	physEngine = new PhysicsEngine(world);
	physEngine->start();
}

void GLSimulation::initializeGL(){
	initializeOpenGLFunctions();
	setMouseTracking(true);
	srand(time(NULL));

	glClearColor(0, 0, 0, 1);

	glEnable(GL_DEPTH_TEST);
	glDepthRange(0.0f, 1.0f);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);

	fpsTimer.setTimerType(Qt::PreciseTimer);
	connect(&fpsTimer, &QTimer::timeout, this, &GLSimulation::frame_tick);
	fpsTimer.start(1000);

	QTimer::singleShot(0, this, &GLSimulation::renderLoop);
}

void GLSimulation::resizeGL(int w, int h){
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, (float)w / h, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void GLSimulation::paintGL(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set background color
	glClearColor(117.0f / 255.0f, 1.0f, 250.0f / 255.0f, 1.0f);

	glLoadIdentity();

	// Scale with zoom factor
	glScalef(zoom, zoom, 1.0f);

	// Move scene relative to camera position
	glRotatef(camera.rotX, 1.0, 0.0, 0.0);
	glRotatef(camera.rotY, 0.0, 1.0, 0.0);
	glTranslated(-camera.x, -camera.y, -camera.z);

	// Set light to static position
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	GLfloat pos[] = { 0.0, 50.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	int nplanes = world.planes.size();
	for (int i = 0; i < nplanes; ++i)
		world.planes[i]->draw();
	int naabbs = world.aabbs.size();
	for (int i = 0; i < naabbs; ++i)
		world.aabbs[i]->draw();
	int nspheres = world.spheres.size();
	for (int i = 0; i < nspheres; ++i)
		world.spheres[i]->draw();

	frames++;
}

void GLSimulation::handleKeyobardEvents(){
	if (keystates[Qt::Key_W]) {
		// Move forward
		camera.x += float(sin(camera.rotY * RAD_PER_DEG)) * camera.v;
		camera.y -= float(sin(camera.rotX * RAD_PER_DEG)) * camera.v;
		camera.z -= float(cos(camera.rotY * RAD_PER_DEG)) * camera.v;
	}
	if (keystates[Qt::Key_S]) {
		// Move backwards
		camera.x -= float(sin(camera.rotY * RAD_PER_DEG)) * camera.v;
		camera.y += float(sin(camera.rotX * RAD_PER_DEG)) * camera.v;
		camera.z += float(cos(camera.rotY * RAD_PER_DEG)) * camera.v;
	}
	if (keystates[Qt::Key_A]) {
		// Move left
		camera.x -= float(cos(camera.rotY * RAD_PER_DEG)) * camera.v;
		camera.z -= float(sin(camera.rotY * RAD_PER_DEG)) * camera.v;
	}
	if (keystates[Qt::Key_D]) {
		// Move right
		camera.x += float(cos(camera.rotY * RAD_PER_DEG)) * camera.v;
		camera.z += float(sin(camera.rotY * RAD_PER_DEG)) * camera.v;
	}
	if (keystates[Qt::Key_Q]) {
		// Move down
		camera.y -= camera.v;
	}
	if (keystates[Qt::Key_E]) {
		// Move up
		camera.y += camera.v;
	}
}

Vec3f GLSimulation::mouseToWorld(int x, int y){
	double yCart = (double)height() - y - 1;

	makeCurrent();

	// Read conversion matrices
	GLint vport[4];
	GLdouble mmat[16];
	GLdouble pmat[16];
	glGetIntegerv(GL_VIEWPORT, vport);
	glGetDoublev(GL_MODELVIEW_MATRIX, mmat);
	glGetDoublev(GL_PROJECTION_MATRIX, pmat);
	
	// Read depth buffer
	float depth;
	glReadPixels(x, yCart, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

	// Convert to world coordinates
	double posX, posY, posZ;
	gluUnProject(x, yCart, depth, mmat, pmat, vport, &posX, &posY, &posZ);

	return Vec3f(posX, posY, posZ);
}

void GLSimulation::keyPressEvent(QKeyEvent* event){
	keystates[event->key()] = true;

	if (event->key() == Qt::Key_Escape) {
		// Exit
		physEngine->stop();
		exit(0);
	}

	if (event->key() == Qt::Key_Space) {
		// Pause/unpause simulation
		physEngine->flip();
	}

	if (event->key() == Qt::Key_R) {
		// Reset all simulated objects
		for (int i = 0; i < world.spheres.size(); ++i)
			world.spheres[i]->reset();
	}

	if (event->key() == Qt::Key_Right) {
		// Step forward by one frame
		physEngine->step();
	}

	if (event->key() == Qt::Key_Z) {
		// Reset zoom
		zoom = 1.0f;
	}

	if (event->key() == Qt::Key_G) {
		// Generate multiple balls with randomized properties
		generateBalls();
	}

	if (event->key() == Qt::Key_Backspace) {
		// Delete selected ball
		if (selected != nullptr) {
			int nspheres = world.spheres.size();
			for (int i = 0; i < nspheres; ++i) {
				Sphere* s = world.spheres[i].get();
				if (s != nullptr && s == selected) {
					world.spheres.erase(world.spheres.begin() + i);
					selected = nullptr;
					break;
				}
			}
		}
	}
}

void GLSimulation::keyReleaseEvent(QKeyEvent* event){
	if (!event->isAutoRepeat()) {
		keystates[event->key()] = false;
	}
}

void GLSimulation::mousePressEvent(QMouseEvent* e){
	keystates[e->button()] = true;

	if (e->button() == Qt::LeftButton) {
		// Select ball
		Vec3f wcoord(mouseToWorld(e->x(), e->y()));
		lastX = e->x(), lastY = e->y();

		// Check for ball selection
		for (int i = 0; i < world.spheres.size(); ++i) {
			Sphere* s = world.spheres[i].get();
			if ((s->pos - wcoord).normsq() <= (double)s->rad * s->rad + 0.02 && s != selected) {
				if (selected != nullptr) { selected->selected = false; }
				s->selected = true;

				// Disable global selection to prevent update due to cyclic trigger
				selected = nullptr;
					
				// These are emitted synchronously so we don't have race condition with the above
				emit massChanged(s->m);
				emit restitutionChanged(s->r);
				emit radiusChanged(s->rad);
				emit xChanged((s->origPos.x + 25) * 2);
				emit yChanged(s->origPos.y * 5);
				emit zChanged((s->origPos.z + 25) * 2);
				emit vxChanged(s->origVelocity.x);
				emit vyChanged(s->origVelocity.y);
				emit vzChanged(s->origVelocity.z);
					
				// Update selection
				selected = s;
				break;
			}
		}
	} else if (e->button() == Qt::RightButton) {
		// Create a new ball at cursor position

		// Coordinates for new ball
		float x = camera.x + float(sin(camera.rotY * RAD_PER_DEG)) * 3;
		float y = camera.y - float(sin(camera.rotX * RAD_PER_DEG)) * 3;
		float z = camera.z - float(cos(camera.rotY * RAD_PER_DEG)) * 3;
		Vec3f newPos(x, y, z);
			
		// Check if new position does not collide with existing balls
		bool foundCollision = false;
		int nspheres = world.spheres.size();
		for (int i = 0; i < nspheres; ++i) {
			Sphere* s = world.spheres[i].get();
			if (s != nullptr) {
				if ((s->pos - newPos).normsq() <= s->rad * s->rad + 0.5 * 0.5 + 0.02) {
					foundCollision = true;
					break;
				}
			}
		}
		if (!foundCollision) {
			if (selected != nullptr) {
				// use values of currently selected ball
				world.spheres.push_back(std::make_unique<Sphere>(
					Sphere(Vec3f(x, y, z), selected->rad, selected->m, selected->r, selected->origVelocity)));
			} else {
				world.spheres.push_back(std::make_unique<Sphere>(Sphere(Vec3f(x, y, z), 0.2, 0.5)));
			}
			
		}
	}
}

void GLSimulation::mouseReleaseEvent(QMouseEvent* e){
	keystates[e->button()] = false;
}

void GLSimulation::mouseMoveEvent(QMouseEvent* e){
	if (keystates[Qt::LeftButton]) {
		// Turn camera
		int diffX = (e->x() - lastX) * 0.2;
		int diffY = (e->y() - lastY) * 0.2;
		lastX = e->x(), lastY = e->y();
		camera.rotY += diffX;
		camera.rotX += diffY;
		if (camera.rotX >= 360) camera.rotX -= 360;
		if (camera.rotX <= -360) camera.rotX += 360;
		if (camera.rotY >= 360) camera.rotY -= 360;
		if (camera.rotY <= -360) camera.rotY += 360;
	}
}

void GLSimulation::wheelEvent(QWheelEvent* event){
	if (event->delta() > 0) {
		zoom += 0.2f;
	} else if (event->delta() < 0) {
		zoom -= 0.2f;
	}
}

void GLSimulation::closeEvent(QCloseEvent* event) {
	physEngine->stop();
}

void GLSimulation::renderLoop() {
	handleKeyobardEvents();
	update();
	QTimer::singleShot(1000.0f / fps, this, &GLSimulation::renderLoop);
}

void GLSimulation::generateBalls(){
	bool physRunning = physEngine->running;
	if (physRunning) physEngine->stop();
	world.spheres.clear();
	for (float x = -5; x < 5; x += 0.5) {
		for (float y = 12; y <= 15; y += 1) {
			for (float z = -5; z < 5; z += 0.5) {
				float mass = 0.4 + (rand() % 600) / 1000.0;
				float restitution = 0.55 + (rand() % 400) / 1000.0;
				float vx = -10.0 + (rand() % 2000) / 100.0;
				float vy = -10.0 + (rand() % 1000) / 100.0;
				float vz = -10.0 + (rand() % 2000) / 100.0;
				world.spheres.push_back(std::make_unique<Sphere>(Sphere(Vec3f(x, y, z), 0.2, mass, restitution, Vec3f(vx, vy, vz))));
			}
		}
	}
	if(physRunning) physEngine->flip();
}

void GLSimulation::frame_tick() {
	#ifdef DEBUG
	qDebug() << "FPS: " << frames;
	#endif
	frames = 0;
}

void GLSimulation::updateMass(double mass) {
	if (selected != nullptr) {
		selected->m = mass;
	}
}

void GLSimulation::updateRestitution(double res){
	if (selected != nullptr) {
		selected->r = res;
	}
}

void GLSimulation::updateRadius(double radius){
	if (selected != nullptr) {
		selected->rad = radius;
	}
}

void GLSimulation::updateX(int x){
	if (selected != nullptr) {
		float newX = (x - 50) * 0.5f;
		selected->origPos.x = newX;
		if (!physEngine->running) {
			selected->pos.x = newX;
		}
	}
}

void GLSimulation::updateY(int y){
	float newY = y * 0.2f;
	if (selected != nullptr && newY >= selected->rad) {
		selected->origPos.y = newY;
		if (!physEngine->running) {
			selected->pos.y = newY;
		}
	}
}

void GLSimulation::updateZ(int z){
	if (selected != nullptr) {
		float newZ = (z - 50) * 0.5f;
		selected->origPos.z = newZ;
		if (!physEngine->running) {
			selected->pos.z = newZ;
		}
	}
}

void GLSimulation::updateVx(double v){
	if (selected != nullptr) {
		selected->origVelocity.x = v;
		if(!physEngine->running)
			selected->velocity.x = v;
	}
}

void GLSimulation::updateVy(double v){
	if (selected != nullptr) {
		selected->origVelocity.y = v;
		if(!physEngine->running)
			selected->velocity.y = v;
	}
}

void GLSimulation::updateVz(double v){
	if (selected != nullptr) {
		selected->origVelocity.z = v;
		if(!physEngine->running)
			selected->velocity.z = v;
	}
}

void GLSimulation::updateCameraSpeed(double v){
	camera.v = v;
}

void GLSimulation::updateFPS(int f){
	fps = f;
}

void GLSimulation::updatePhysicsFPS(int physFPS){
	physEngine->fps = physFPS;
}

void GLSimulation::animationButtonPressed(){
	physEngine->flip();
}

void GLSimulation::stepButtonPressed(){
	physEngine->step();
}

void GLSimulation::resetCurrentButtonPressed(){
	if (selected != nullptr) {
		selected->reset();
	}
}

void GLSimulation::resetAllButtonPressed(){
	for (int i = 0; i < world.spheres.size(); ++i)
		world.spheres[i]->reset();
}

void GLSimulation::clearAllButtonPressed(){
	bool physRunning = physEngine->running;
	if(physRunning) physEngine->stop();
	world.spheres.clear();
	selected = nullptr;
	if(physRunning) physEngine->flip();
}

void GLSimulation::addExternalForce(Vec3f& dir, float power, float decay){
	if(selected != nullptr){
		selected->forces.push_back(Force(dir, power, decay));
	}
}

void GLSimulation::switchWallsButtonPressed(bool state){
	if (state) {
		// left wall
		world.aabbs.push_back(std::make_unique<AABB>(AABB(Vec3f(-30, 0, 30), Vec3f(-30, 0, -30), Vec3f(-30, 15, -30), Vec3f(-30, 15, 30), Vec3f(0.5, 0.4, 0.8))));
		// right wall
		world.aabbs.push_back(std::make_unique<AABB>(AABB(Vec3f(30, 0, 30), Vec3f(30, 15, 30), Vec3f(30, 15, -30), Vec3f(30, 0, -30), Vec3f(0.5, 0.4, 0.8))));
		// back wall
		world.aabbs.push_back(std::make_unique<AABB>(AABB(Vec3f(30, 0, -30), Vec3f(30, 15, -30), Vec3f(-30, 15, -30), Vec3f(-30, 0, -30), Vec3f(0.5, 0.4, 0.8))));
		// front wall
		world.aabbs.push_back(std::make_unique<AABB>(AABB(Vec3f(-30, 0, 30), Vec3f(-30, 15, 30), Vec3f(30, 15, 30), Vec3f(30, 0, 30), Vec3f(0.5, 0.4, 0.8))));
	} else {
		world.aabbs.clear();
	}
}

void GLSimulation::randomizeButtonPressed(){
	generateBalls();
}
