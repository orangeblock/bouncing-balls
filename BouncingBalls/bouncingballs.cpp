#include "bouncingballs.h"
#include <QOpenGLWidget>

BouncingBalls::BouncingBalls(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(this, &BouncingBalls::forceApplied, ui.openGLWidget, &GLSimulation::addExternalForce);
}

void BouncingBalls::applyForcePressed() {
	Vec3f dir(ui.dial->value(), ui.dial_2->value(), ui.dial_3->value());
	emit forceApplied(dir, ui.doubleSpinBox_9->value(), ui.doubleSpinBox_4->value());
}

void BouncingBalls::closeEvent(QCloseEvent* event) {
	findChild<QOpenGLWidget*>()->close();
}
