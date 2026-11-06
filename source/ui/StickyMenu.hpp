#pragma once

#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPointer>
#include <QTimer>

namespace Munin::UI {

/**
 * @brief Menu aderente com rastreamento contínuo de ponteiro de mouse e
 * propagação de fechamento seguro, no padrão nativo do projeto Linceu_Lighthouse_cpp.
 */
class StickyMenu : public QMenu {
    Q_OBJECT

public:
    explicit StickyMenu(const QString &title, QWidget *parent = nullptr);
    ~StickyMenu() override = default;

    void setStickyParent(StickyMenu *parentMenu) { m_stickyParentMenu = parentMenu; }
    StickyMenu *stickyParent() const { return m_stickyParentMenu.data(); }

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void armCloseOnMouseExit();
    void armCloseOnMouseExitChain();
    void disarmCloseOnMouseExit();
    void updateCloseOnMouseExit();

    static bool cursorOverMenuTree(const StickyMenu *menu);

    QPointer<StickyMenu> m_stickyParentMenu;
    QPointer<StickyMenu> m_closeBranchRoot;
    QTimer m_closeWatcher;
    bool m_closeOnMouseExit = false;
};

// Funções auxiliares para construção hierárquica e exibição de menus aderentes
StickyMenu *create_menu(const QString &title, QWidget *parent = nullptr);
void add_submenu(QMenu *parentMenu, StickyMenu *submenu);
void popup_menu(QMenu *menu, QMenuBar *menuBar);

} // namespace Munin::UI
