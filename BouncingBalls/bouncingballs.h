#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_bouncingballs.h"

class BouncingBalls : public QMainWindow
{
	Q_OBJECT

public:
	BouncingBalls(QWidget *parent = Q_NULLPTR);

signals:
	void forceApplied(Vec3f& dir, float power, float decay);

public slots:
	void applyForcePressed();

private:
	Ui::BouncingBallsClass ui;
};
