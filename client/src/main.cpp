#include <QApplication>
#include "MainWindow/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setStyle("Fusion");

    MainWindow window;
    window.resize(1920, 1080);
    window.show();

    return app.exec();
}