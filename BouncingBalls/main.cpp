#include "bouncingballs.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	BouncingBalls w;
	w.show();
	return a.exec();
}
