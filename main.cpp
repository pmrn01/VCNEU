#include <QApplication>
#include "paint.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
    Paint w;
	w.show();
	return app.exec();
}
