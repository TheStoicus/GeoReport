#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("TerraNex");
    QApplication::setApplicationVersion("0.1.0");
    QApplication::setOrganizationName("TerraNex");

    MainWindow window;
    window.show();

    return app.exec();
}
