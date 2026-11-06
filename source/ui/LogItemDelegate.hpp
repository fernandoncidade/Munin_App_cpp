#pragma once

#include "../model/LogModel.hpp"

#include <QStyledItemDelegate>

namespace Munin::UI {

class LogItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit LogItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setSearchHighlight(const QString& query, bool isRegex, bool caseSensitive);
    void clearSearchHighlight();

    void setDarkTheme(bool isDark) { m_isDark = isDark; }
    [[nodiscard]] bool isDarkTheme() const noexcept { return m_isDark; }

private:
    QString m_searchQuery;
    bool m_isRegex{false};
    bool m_caseSensitive{false};
    bool m_isDark{true};
};

} // namespace Munin::UI
