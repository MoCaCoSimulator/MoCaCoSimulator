#include "MainWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication a(argc, argv);
	MainWindow w;
	//w.resize(1280, 728);
	w.show();
	return a.exec();
}