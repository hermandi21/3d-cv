//
// (c) Georg Umlauf, 2021
//
#include "mainwindow.h"

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    //Update from BasicGeometry
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
