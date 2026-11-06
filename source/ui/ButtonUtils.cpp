#include "ButtonUtils.hpp"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>

namespace Munin::UI {

void focus_license_action_button(QPushButton *button) {
    if (button == nullptr || !button->isEnabled()) {
        return;
    }
    button->setFocus();
}

void select_license_action_button(QPushButton *button) {
    if (button == nullptr || !button->isEnabled()) {
        return;
    }
    button->setDefault(true);
    button->setFocus();
}

void configure_license_action_button(QPushButton *button, int minWidth) {
    if (button == nullptr) {
        return;
    }

    if (minWidth > 0) {
        button->setMinimumWidth(minWidth);
    }
    button->setAutoDefault(false);
    button->setDefault(false);
}

void configure_tool_button(QToolButton *button, Qt::ArrowType arrowType, const QString &tooltip) {
    if (button == nullptr) {
        return;
    }

    button->setArrowType(arrowType);
    button->setAutoRaise(true);
    if (!tooltip.isEmpty()) {
        button->setToolTip(tooltip);
        button->setAccessibleName(tooltip);
    }
}

void configure_document_tabs(QTabWidget *document_tabs) {
    if (document_tabs == nullptr) {
        return;
    }
    document_tabs->setDocumentMode(true);
    document_tabs->setUsesScrollButtons(true);
    document_tabs->setElideMode(Qt::ElideNone);
}

void apply_window_bg(QWidget *widget, const QColor &color) {
    if (widget == nullptr) {
        return;
    }

    QPalette palette = widget->palette();
    palette.setColor(QPalette::Window, color);
    widget->setAutoFillBackground(true);
    widget->setPalette(palette);
}

void apply_list_bg(QWidget *widget, const QColor &color) {
    if (widget == nullptr) {
        return;
    }

    QPalette palette = widget->palette();
    palette.setColor(QPalette::Base, color);
    palette.setColor(QPalette::AlternateBase, color);
    if (qApp) {
        palette.setColor(QPalette::Text, qApp->palette().color(QPalette::Text));
        palette.setColor(QPalette::WindowText, qApp->palette().color(QPalette::WindowText));
        palette.setColor(QPalette::Highlight, qApp->palette().color(QPalette::Highlight));
        palette.setColor(QPalette::HighlightedText, qApp->palette().color(QPalette::HighlightedText));
    }
    widget->setPalette(palette);
    widget->setAutoFillBackground(true);
    if (auto *view = qobject_cast<QAbstractItemView *>(widget)) {
        if (view->viewport() != nullptr) {
            view->viewport()->setAutoFillBackground(true);
            view->viewport()->setPalette(palette);
        }
    }
}

void apply_input_bg(QWidget *widget, const QColor &color) {
    if (widget == nullptr) {
        return;
    }

    QPalette palette = widget->palette();
    palette.setColor(QPalette::Base, color);
    palette.setColor(QPalette::AlternateBase, color);
    if (qApp) {
        palette.setColor(QPalette::Text, qApp->palette().color(QPalette::Text));
        palette.setColor(QPalette::WindowText, qApp->palette().color(QPalette::WindowText));
        palette.setColor(QPalette::PlaceholderText, qApp->palette().color(QPalette::PlaceholderText));
    }

    if (auto *combo = qobject_cast<QComboBox *>(widget)) {
        palette.setColor(QPalette::Button, color);
        palette.setColor(QPalette::Window, color);
        combo->setPalette(palette);
        if (combo->view() != nullptr) {
            QPalette viewPalette = combo->view()->palette();
            viewPalette.setColor(QPalette::Base, color);
            viewPalette.setColor(QPalette::AlternateBase, color);
            if (qApp) {
                viewPalette.setColor(QPalette::Text, qApp->palette().color(QPalette::Text));
                viewPalette.setColor(QPalette::WindowText, qApp->palette().color(QPalette::WindowText));
            }
            combo->view()->setPalette(viewPalette);
        }
    } else {
        widget->setPalette(palette);
    }
}

void apply_document_base_background(QWidget *widget, const QPalette &sourcePalette) {
    if (widget == nullptr) {
        return;
    }

    QPalette palette = sourcePalette;
    const QColor base = sourcePalette.color(QPalette::Base);
    for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        palette.setColor(group, QPalette::Window, base);
        palette.setColor(group, QPalette::Base, base);
        palette.setColor(group, QPalette::AlternateBase, base);
    }
    widget->setPalette(palette);
    widget->setAutoFillBackground(true);
}

void refresh_document_base_backgrounds(QWidget *root, const QPalette &sourcePalette) {
    if (root == nullptr) {
        return;
    }
    apply_document_base_background(root, sourcePalette);
}

} // namespace Munin::UI
