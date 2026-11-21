#include "mainwindow.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QFont>
#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Request Per-Monitor V2 scaling before Qt creates the first native window.
    (void)SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Ivan-VHS");
    QCoreApplication::setApplicationName("VentuSample");
    QCoreApplication::setApplicationVersion("0.1.0");
    QApplication::setFont(QFont("Segoe UI", 10));
    MainWindow window;
    window.show();
    return app.exec();
}
