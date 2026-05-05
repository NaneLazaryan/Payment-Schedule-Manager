#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application metadata
    app.setApplicationName("Recurring Payment Schedule Manager");

    MainWindow window;
    window.show();

    return app.exec();
}
