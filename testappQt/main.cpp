#include "TrianglePPDemoApp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TrianglePPDemoApp w;
    w.show();

    return a.exec();
}
