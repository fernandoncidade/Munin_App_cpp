// Munin App - Leitor de Logs Massivos
// Forwarding entry point to source/main.cpp
#include "source/ui/MainWindow.hpp"

#include <QApplication>


int main(int argc, char* argv[]) {
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Munin App"));
    app.setApplicationDisplayName(QString());
app.setApplicationVersion(QStringLiteral("2026.11.6.0"));
    app.setOrganizationName(QStringLiteral("Munin App"));



    Munin::UI::MainWindow mainWindow;
    mainWindow.show();

    if (argc > 1) {
        QString initialFile = QString::fromLocal8Bit(argv[1]);
        mainWindow.openLogFile(initialFile);
    }

    return app.exec();
}
