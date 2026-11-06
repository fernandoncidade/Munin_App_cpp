#include "ui/MainWindow.hpp"
#include "ui/ThemeManager.hpp"
#include "utils/ApplicationPathUtils.hpp"
#include "utils/LogManager.hpp"

#include <QApplication>
#include <QIcon>

#if defined(Q_OS_WIN)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shobjidl.h>
#endif

int main(int argc, char* argv[]) {
    // Explicitly configure Windows taskbar model ID
#if defined(Q_OS_WIN)
    SetCurrentProcessExplicitAppUserModelID(L"Munin.LogViewer.App.v1");
#endif

    // Initialize logging
    LogManager::configure_logging();
    LogManager::info(QStringLiteral("=== Iniciando Munin App v2026.11.6.0 ==="));

    // Enable High DPI scaling
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Munin App"));
    app.setApplicationDisplayName(QString());
app.setApplicationVersion(QStringLiteral("2026.11.6.0"));
    app.setOrganizationName(QStringLiteral("Munin App"));

    // Set application window and taskbar icon
    const QIcon appIcon = get_app_icon();
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    }

// Native Qt6 default theme (no Fusion)

    Munin::UI::MainWindow mainWindow;
    mainWindow.show();

    // Initialize dynamic theme manager (supports Windows 11 system theme transitions)
    Munin::UI::ThemeManager::instance().initialize(&mainWindow);

    // If a file argument was passed on command line, open it immediately
    if (argc > 1) {
        QString initialFile = QString::fromLocal8Bit(argv[1]);
        mainWindow.openLogFile(initialFile);
    }

    return app.exec();
}
