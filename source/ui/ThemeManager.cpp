#include "ThemeManager.hpp"
#include "Win32DwmHelper.hpp"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QStyleHints>
#include <QTimer>

#if defined(Q_OS_WIN)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace Munin::UI {

ThemeManager& ThemeManager::instance() {
    static ThemeManager s_instance;
    return s_instance;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent) {
}

ThemeManager::~ThemeManager() {
    if (auto* app = QCoreApplication::instance()) {
        app->removeNativeEventFilter(this);
    }
}

void ThemeManager::initialize(QWidget* mainWindow) {
    if (mainWindow) {
        m_mainWindow = mainWindow;
    }

    if (auto* app = QCoreApplication::instance()) {
        app->installNativeEventFilter(this);
    }

    if (!m_pollTimer) {
        m_pollTimer = new QTimer(this);
        m_pollTimer->setInterval(500);
        connect(m_pollTimer, &QTimer::timeout, this, &ThemeManager::checkSystemThemePoll);
        m_pollTimer->start();
    }

    // Conectar ao sinal nativo de mudança de esquema de cores do Qt 6.5+
    if (auto* hints = QGuiApplication::styleHints()) {
        connect(hints, &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme) {
            onSystemColorSchemeChanged();
        });
    }

    m_initialized = true;
    updateTheme(true);
}

bool ThemeManager::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) {
    Q_UNUSED(eventType);
    Q_UNUSED(result);
#if defined(Q_OS_WIN)
    MSG* msg = static_cast<MSG*>(message);
    if (msg && msg->message == WM_SETTINGCHANGE) {
        checkSystemThemePoll();
    }
#endif
    return false;
}

void ThemeManager::setThemeMode(ThemeMode /*mode*/) {
    updateTheme(true);
}

bool ThemeManager::isDark() const {
    return Win32DwmHelper::isSystemDarkMode();
}

void ThemeManager::onSystemColorSchemeChanged() {
    updateTheme();
}

void ThemeManager::checkSystemThemePoll() {
    bool dark = Win32DwmHelper::isSystemDarkMode();
    if (dark != m_lastAppliedDark) {
        updateTheme();
    }
}

void ThemeManager::refreshTheme() {
    checkSystemThemePoll();
}

void ThemeManager::applyGlobalScrollbarStyle(bool isDark) {
    if (!qApp) return;

    const QString thumbColor = isDark ? QStringLiteral("rgba(255, 255, 255, 0.22)")
                                      : QStringLiteral("rgba(0, 0, 0, 0.22)");
    const QString thumbHover = isDark ? QStringLiteral("rgba(255, 255, 255, 0.38)")
                                      : QStringLiteral("rgba(0, 0, 0, 0.38)");
    const QString thumbPressed = isDark ? QStringLiteral("rgba(255, 255, 255, 0.50)")
                                        : QStringLiteral("rgba(0, 0, 0, 0.50)");
    const QString trackColor = QStringLiteral("transparent");

    const QString scrollbarQss = QStringLiteral(
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: %1;"
        "    width: 10px;"
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: %2;"
        "    min-height: 24px;"
        "    border-radius: 4px;"
        "    margin: 2px 2px 2px 2px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: %3;"
        "}"
        "QScrollBar::handle:vertical:pressed {"
        "    background: %4;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    border: none;"
        "    background: none;"
        "    height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
        "QScrollBar:horizontal {"
        "    border: none;"
        "    background: %1;"
        "    height: 10px;"
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "    background: %2;"
        "    min-width: 24px;"
        "    border-radius: 4px;"
        "    margin: 2px 2px 2px 2px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "    background: %3;"
        "}"
        "QScrollBar::handle:horizontal:pressed {"
        "    background: %4;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "    border: none;"
        "    background: none;"
        "    width: 0px;"
        "}"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
        "    background: none;"
        "}"
    ).arg(trackColor, thumbColor, thumbHover, thumbPressed);

    qApp->setStyleSheet(scrollbarQss);
}

void ThemeManager::updateTheme(bool forceNotify) {
    if (m_isUpdating) return;
    m_isUpdating = true;

    bool dark = isDark();

    if (forceNotify || dark != m_lastAppliedDark || !m_initialized) {
        m_lastAppliedDark = dark;

        // 1. Aplica Paleta Nativa do Qt (mesma do Linceu_Lighthouse_cpp)
        if (qApp) {
            qApp->setPalette(dark ? createDarkPalette() : createLightPalette());
            applyGlobalScrollbarStyle(dark);
        }

        // 2. Aplica Tema na Barra de Títulos DWM do Windows 11 (Mica / Dark Mode)
        if (m_mainWindow) {
            Win32DwmHelper::applyTitleBarTheme(m_mainWindow, dark);
        }

        // 3. Notifica widgets registrados para sincronização de estados
        emit themeChanged(dark);

        if (m_mainWindow) {
            m_mainWindow->update();
        }
    }

    m_isUpdating = false;
}

QPalette ThemeManager::createDarkPalette() const {
    QPalette pal;

    auto set_group = [](QPalette& p,
                        QPalette::ColorGroup g,
                        const QColor& window,
                        const QColor& base,
                        const QColor& alternate_base,
                        const QColor& text,
                        const QColor& button,
                        const QColor& button_text,
                        const QColor& mid,
                        const QColor& dark,
                        const QColor& light,
                        const QColor& shadow,
                        const QColor& highlight,
                        const QColor& highlighted_text) {
        p.setColor(g, QPalette::Window, window);
        p.setColor(g, QPalette::Base, base);
        p.setColor(g, QPalette::AlternateBase, alternate_base);
        p.setColor(g, QPalette::Text, text);
        p.setColor(g, QPalette::WindowText, text);
        p.setColor(g, QPalette::Button, button);
        p.setColor(g, QPalette::ButtonText, button_text);
        p.setColor(g, QPalette::Mid, mid);
        p.setColor(g, QPalette::Dark, dark);
        p.setColor(g, QPalette::Light, light);
        p.setColor(g, QPalette::Shadow, shadow);
        p.setColor(g, QPalette::Highlight, highlight);
        p.setColor(g, QPalette::HighlightedText, highlighted_text);
        p.setColor(g, QPalette::ToolTipBase, base);
        p.setColor(g, QPalette::ToolTipText, text);
        p.setColor(g, QPalette::PlaceholderText, QColor(145, 145, 150));
    };

    const QColor window(30, 30, 32);
    const QColor base(34, 34, 36);
    const QColor alt(44, 44, 46);
    const QColor text(235, 235, 235);
    const QColor button(48, 48, 52);
    const QColor button_text(235, 235, 235);
    const QColor mid(96, 96, 102);
    const QColor dark(22, 22, 24);
    const QColor light(78, 78, 84);
    const QColor shadow(12, 12, 14);
    const QColor highlight(82, 133, 255);
    const QColor highlighted_text(255, 255, 255);

    set_group(pal, QPalette::Active, window, base, alt, text, button, button_text, mid, dark, light, shadow, highlight, highlighted_text);
    set_group(pal, QPalette::Inactive, window, base, alt, text, button, button_text, mid, dark, light, shadow, highlight, highlighted_text);
    set_group(pal, QPalette::Disabled, window, base, alt, QColor(145, 145, 150), button, QColor(145, 145, 150), mid, dark, light, shadow, QColor(92, 92, 96), QColor(210, 210, 210));

    return pal;
}

QPalette ThemeManager::createLightPalette() const {
    QPalette pal;

    auto set_group = [](QPalette& p,
                        QPalette::ColorGroup g,
                        const QColor& window,
                        const QColor& base,
                        const QColor& alternate_base,
                        const QColor& text,
                        const QColor& button,
                        const QColor& button_text,
                        const QColor& mid,
                        const QColor& dark,
                        const QColor& light,
                        const QColor& shadow,
                        const QColor& highlight,
                        const QColor& highlighted_text) {
        p.setColor(g, QPalette::Window, window);
        p.setColor(g, QPalette::Base, base);
        p.setColor(g, QPalette::AlternateBase, alternate_base);
        p.setColor(g, QPalette::Text, text);
        p.setColor(g, QPalette::WindowText, text);
        p.setColor(g, QPalette::Button, button);
        p.setColor(g, QPalette::ButtonText, button_text);
        p.setColor(g, QPalette::Mid, mid);
        p.setColor(g, QPalette::Dark, dark);
        p.setColor(g, QPalette::Light, light);
        p.setColor(g, QPalette::Shadow, shadow);
        p.setColor(g, QPalette::Highlight, highlight);
        p.setColor(g, QPalette::HighlightedText, highlighted_text);
        p.setColor(g, QPalette::ToolTipBase, base);
        p.setColor(g, QPalette::ToolTipText, text);
        p.setColor(g, QPalette::PlaceholderText, QColor(120, 120, 120));
    };

    const QColor window(250, 250, 250);
    const QColor base(255, 255, 255);
    const QColor alt(247, 247, 247);
    const QColor text(20, 20, 20);
    const QColor button(243, 243, 243);
    const QColor button_text(20, 20, 20);
    const QColor mid(191, 195, 201);
    const QColor dark(159, 165, 173);
    const QColor light(255, 255, 255);
    const QColor shadow(115, 121, 129);
    const QColor highlight(40, 120, 220);
    const QColor highlighted_text(255, 255, 255);

    set_group(pal, QPalette::Active, window, base, alt, text, button, button_text, mid, dark, light, shadow, highlight, highlighted_text);
    set_group(pal, QPalette::Inactive, window, base, alt, text, button, button_text, mid, dark, light, shadow, highlight, highlighted_text);
    set_group(pal, QPalette::Disabled, window, base, alt, QColor(120, 120, 120), button, QColor(120, 120, 120), mid, dark, light, shadow, QColor(145, 170, 210), QColor(245, 245, 245));

    return pal;
}

} // namespace Munin::UI
