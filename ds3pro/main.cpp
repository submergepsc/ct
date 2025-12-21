#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.setWindowTitle(QStringLiteral("教学计划编排"));
    window.resize(520, 400);
    window.show();

    return app.exec();
}
