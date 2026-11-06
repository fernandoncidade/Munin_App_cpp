#pragma once

#include <QWidget>
#include <QString>

namespace Munin::UI {

class Win32DwmHelper {
public:
    // Detects whether Windows 11 / Windows 10 is currently configured for dark mode
    static bool isSystemDarkMode();

    // Applies Windows 11 immersive dark mode (or light mode) and Mica backdrop effect to window frame
    static void applyTitleBarTheme(QWidget* widget, bool isDark);

    // Backward compatibility helper that queries isSystemDarkMode()
    static void applyMicaAndDarkMode(QWidget* widget);

    // Formats memory usage in MB / GB
    static QString getProcessMemoryUsageString();
};

} // namespace Munin::UI
