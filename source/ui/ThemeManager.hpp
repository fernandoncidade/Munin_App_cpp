#pragma once

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QPalette>
#include <QWidget>

namespace Munin::UI {

enum class ThemeMode {
    System
};

/**
 * @brief Gerenciador nativo de temas e paletas de cores do Qt6,
 * com suporte à transição em tempo de execução para Windows 11 (Automático: Dark / Light)
 * e alinhamento com a abordagem por QPalette do Linceu_Lighthouse_cpp.
 */
class ThemeManager : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT

public:
    static ThemeManager& instance();

    void initialize(QWidget* mainWindow);
    void setThemeMode(ThemeMode mode = ThemeMode::System);
    ThemeMode currentMode() const { return ThemeMode::System; }

    bool isDark() const;
    void refreshTheme();
    void checkSystemThemePoll();

    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

    QPalette createDarkPalette() const;
    QPalette createLightPalette() const;
    void applyGlobalScrollbarStyle(bool isDark);

signals:
    void themeChanged(bool isDark);

private:
    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager() override;

    void onSystemColorSchemeChanged();
    void updateTheme(bool forceNotify = false);

    QWidget* m_mainWindow = nullptr;
    class QTimer* m_pollTimer = nullptr;
    bool m_lastAppliedDark = false;
    bool m_initialized = false;
    bool m_isUpdating = false;
};

} // namespace Munin::UI
