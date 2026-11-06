#include "Win32DwmHelper.hpp"

#include <QGuiApplication>
#include <QStyleHints>
#include <QPalette>

#if defined(Q_OS_WIN)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dwmapi.h>
#include <psapi.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#ifndef DWMSBT_MAINWINDOW
#define DWMSBT_MAINWINDOW 2 // Mica backdrop
#endif
#endif

namespace Munin::UI {

bool Win32DwmHelper::isSystemDarkMode() {
#if defined(Q_OS_WIN)
    // 1. Primary authoritative source on Windows 10/11: Registry AppsUseLightTheme
    HKEY hKey = nullptr;
    if (::RegOpenKeyExW(HKEY_CURRENT_USER,
                        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD appsUseLightTheme = 1;
        DWORD dataSize = sizeof(appsUseLightTheme);
        DWORD type = REG_DWORD;
        LONG status = ::RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, &type,
                                         reinterpret_cast<LPBYTE>(&appsUseLightTheme), &dataSize);
        ::RegCloseKey(hKey);
        if (status == ERROR_SUCCESS) {
            return (appsUseLightTheme == 0);
        }
    }
#endif

    // 2. Check Qt 6.5+ styleHints (fallback)
    if (auto* hints = QGuiApplication::styleHints()) {
        auto scheme = hints->colorScheme();
        if (scheme == Qt::ColorScheme::Dark) return true;
        if (scheme == Qt::ColorScheme::Light) return false;
    }

    // 3. Fallback to palette lightness
    if (qApp) {
        return qApp->palette().color(QPalette::Window).lightness() < 128;
    }
    return true;
}

void Win32DwmHelper::applyTitleBarTheme(QWidget* widget, bool isDark) {
    if (!widget) return;

#if defined(Q_OS_WIN)
    if (!widget->isWindow()) return;
    if (!widget->testAttribute(Qt::WA_WState_Created)) return;

    HWND hwnd = reinterpret_cast<HWND>(widget->winId());
    if (!hwnd) return;

    // Enable/Disable Windows 11 / 10 Immersive Dark Mode for window title bar
    BOOL darkMode = isDark ? TRUE : FALSE;
    ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

    // Enable Mica Backdrop on Windows 11 (build 22621+)
    DWORD backdropType = DWMSBT_MAINWINDOW;
    ::DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));

    // Force immediate non-client title bar and border redraw on Windows 11
    ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

    // Force immediate redraw of entire window client area and children
    ::RedrawWindow(hwnd, nullptr, nullptr,
                   RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_FRAME | RDW_UPDATENOW);
#else
    Q_UNUSED(widget);
    Q_UNUSED(isDark);
#endif
}

void Win32DwmHelper::applyMicaAndDarkMode(QWidget* widget) {
    applyTitleBarTheme(widget, isSystemDarkMode());
}

QString Win32DwmHelper::getProcessMemoryUsageString() {
#if defined(Q_OS_WIN)
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    if (::GetProcessMemoryInfo(
            ::GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
            sizeof(pmc))) {
        double workingSetMB = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
        return QStringLiteral("%1 MB").arg(workingSetMB, 0, 'f', 1);
    }
#endif
    return QStringLiteral("N/A");
}

} // namespace Munin::UI
