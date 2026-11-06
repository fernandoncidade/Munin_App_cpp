#include "StickyMenu.hpp"

#include <QAction>
#include <QCursor>

namespace Munin::UI {

StickyMenu::StickyMenu(const QString &title, QWidget *parent)
    : QMenu(title, parent)
{
    m_closeWatcher.setInterval(50);
    m_closeWatcher.setSingleShot(false);
    connect(&m_closeWatcher, &QTimer::timeout, this, [this]() {
        updateCloseOnMouseExit();
    });
}

void StickyMenu::mouseReleaseEvent(QMouseEvent *event) {
    if (!event) {
        QMenu::mouseReleaseEvent(event);
        return;
    }

    QAction *action = actionAt(event->pos());
    if (event->button() != Qt::LeftButton || !action || !action->isEnabled() || action->isSeparator() || action->menu() != nullptr) {
        QMenu::mouseReleaseEvent(event);
        return;
    }

    setActiveAction(action);
    action->trigger();
    armCloseOnMouseExitChain();
    event->accept();
}

void StickyMenu::hideEvent(QHideEvent *event) {
    disarmCloseOnMouseExit();
    QMenu::hideEvent(event);
}

void StickyMenu::armCloseOnMouseExit() {
    m_closeOnMouseExit = true;
    if (isVisible() && !m_closeWatcher.isActive()) {
        m_closeWatcher.start();
    }
}

void StickyMenu::armCloseOnMouseExitChain() {
    StickyMenu *branchRoot = this;
    while (branchRoot->m_stickyParentMenu != nullptr) {
        branchRoot = branchRoot->m_stickyParentMenu.data();
    }

    for (StickyMenu *menu = this; menu != nullptr; menu = menu->m_stickyParentMenu.data()) {
        menu->m_closeBranchRoot = branchRoot;
        menu->armCloseOnMouseExit();
    }
}

void StickyMenu::disarmCloseOnMouseExit() {
    m_closeOnMouseExit = false;
    m_closeBranchRoot.clear();
    m_closeWatcher.stop();
}

void StickyMenu::updateCloseOnMouseExit() {
    if (!m_closeOnMouseExit) {
        m_closeWatcher.stop();
        return;
    }
    if (!isVisible()) {
        disarmCloseOnMouseExit();
        return;
    }
    StickyMenu *branchRoot = m_closeBranchRoot.data();
    if (cursorOverMenuTree(branchRoot != nullptr ? branchRoot : this)) {
        return;
    }

    disarmCloseOnMouseExit();
    close();
}

bool StickyMenu::cursorOverMenuTree(const StickyMenu *menu) {
    if (!menu || !menu->isVisible()) {
        return false;
    }

    const QPoint cursorPos = QCursor::pos();
    if (menu->frameGeometry().contains(cursorPos)) {
        return true;
    }

    for (QAction *action : menu->actions()) {
        QMenu *submenu = action != nullptr ? action->menu() : nullptr;
        if (!submenu || !submenu->isVisible()) {
            continue;
        }

        if (auto *stickySubmenu = dynamic_cast<StickyMenu *>(submenu)) {
            if (cursorOverMenuTree(stickySubmenu)) {
                return true;
            }
            continue;
        }

        if (submenu->frameGeometry().contains(cursorPos)) {
            return true;
        }
    }

    return false;
}

StickyMenu *create_menu(const QString &title, QWidget *parent) {
    return new StickyMenu(title, parent);
}

void add_submenu(QMenu *parentMenu, StickyMenu *submenu) {
    if (!parentMenu || !submenu) {
        return;
    }

    if (auto *stickyParent = dynamic_cast<StickyMenu *>(parentMenu)) {
        submenu->setStickyParent(stickyParent);
    }

    parentMenu->addMenu(submenu);
}

void popup_menu(QMenu *menu, QMenuBar *menuBar) {
    if (!menu || !menuBar) {
        return;
    }
    const QRect geometry = menuBar->actionGeometry(menu->menuAction());
    menu->popup(menuBar->mapToGlobal(geometry.bottomLeft()));
}

} // namespace Munin::UI
