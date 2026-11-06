#include "LogItemDelegate.hpp"

#include <QPainter>
#include <QRegularExpression>

namespace Munin::UI {

LogItemDelegate::LogItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

void LogItemDelegate::setSearchHighlight(const QString& query, bool isRegex, bool caseSensitive) {
    m_searchQuery = query;
    m_isRegex = isRegex;
    m_caseSensitive = caseSensitive;
}

void LogItemDelegate::clearSearchHighlight() {
    m_searchQuery.clear();
}

QSize LogItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(100, 22);
}

void LogItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // Row Background
    bool isSelected = (opt.state & QStyle::State_Selected) != 0;
    bool isEven = (index.row() % 2 == 0);

    QColor bgColor;
    if (m_isDark) {
        if (isSelected) {
            bgColor = QColor(39, 39, 42); // Zinc 800
        } else if (isEven) {
            bgColor = QColor(18, 18, 20);
        } else {
            bgColor = QColor(24, 24, 27); // Zinc 900
        }
    } else {
        if (isSelected) {
            bgColor = QColor(226, 232, 240); // Slate 200
        } else if (isEven) {
            bgColor = QColor(255, 255, 255);
        } else {
            bgColor = QColor(248, 250, 252); // Slate 50
        }
    }

    painter->fillRect(opt.rect, bgColor);

    if (isSelected) {
        // Subtle left highlight line
        if (index.column() == 0) {
            painter->fillRect(QRect(opt.rect.left(), opt.rect.top(), 3, opt.rect.height()), QColor(2, 132, 199));
        }
    }

    int col = index.column();

    if (col == Model::LogModel::ColLineNumber) {
        painter->setFont(opt.font);
        painter->setPen(m_isDark ? QColor(113, 113, 122) : QColor(100, 116, 139)); // Zinc 500 / Slate 500
        QRect textRect = opt.rect.adjusted(4, 0, -8, 0);
        painter->drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, opt.text);
    } else if (col == Model::LogModel::ColTimestamp) {
        painter->setFont(opt.font);
        painter->setPen(m_isDark ? QColor(148, 163, 184) : QColor(71, 85, 105)); // Slate 400 / Slate 600
        QRect textRect = opt.rect.adjusted(6, 0, -4, 0);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, opt.text);
    } else if (col == Model::LogModel::ColLevel) {
        auto lvl = static_cast<Model::LogLevel>(index.data(Model::LogModel::LogLevelRole).toInt());
        if (lvl != Model::LogLevel::Unknown && !opt.text.isEmpty()) {
            QColor pillBg;
            QColor pillBorder;
            QColor pillText;

            if (m_isDark) {
                switch (lvl) {
                case Model::LogLevel::Fatal:
                case Model::LogLevel::Error:
                    pillBg = QColor(69, 10, 10, 220);
                    pillBorder = QColor(153, 27, 27);
                    pillText = QColor(248, 113, 113);
                    break;
                case Model::LogLevel::Warn:
                    pillBg = QColor(69, 26, 3, 220);
                    pillBorder = QColor(146, 64, 14);
                    pillText = QColor(251, 191, 36);
                    break;
                case Model::LogLevel::Info:
                    pillBg = QColor(8, 47, 73, 220);
                    pillBorder = QColor(3, 105, 161);
                    pillText = QColor(56, 189, 248);
                    break;
                case Model::LogLevel::Debug:
                    pillBg = QColor(46, 16, 101, 220);
                    pillBorder = QColor(88, 28, 135);
                    pillText = QColor(192, 132, 252);
                    break;
                case Model::LogLevel::Trace:
                default:
                    pillBg = QColor(30, 41, 59, 220);
                    pillBorder = QColor(51, 65, 85);
                    pillText = QColor(148, 163, 184);
                    break;
                }
            } else {
                switch (lvl) {
                case Model::LogLevel::Fatal:
                case Model::LogLevel::Error:
                    pillBg = QColor(254, 226, 226);
                    pillBorder = QColor(252, 165, 165);
                    pillText = QColor(185, 28, 28);
                    break;
                case Model::LogLevel::Warn:
                    pillBg = QColor(254, 243, 199);
                    pillBorder = QColor(252, 211, 77);
                    pillText = QColor(180, 83, 9);
                    break;
                case Model::LogLevel::Info:
                    pillBg = QColor(224, 242, 254);
                    pillBorder = QColor(125, 211, 252);
                    pillText = QColor(3, 105, 161);
                    break;
                case Model::LogLevel::Debug:
                    pillBg = QColor(243, 232, 255);
                    pillBorder = QColor(216, 180, 254);
                    pillText = QColor(126, 34, 206);
                    break;
                case Model::LogLevel::Trace:
                default:
                    pillBg = QColor(241, 245, 249);
                    pillBorder = QColor(203, 213, 225);
                    pillText = QColor(71, 85, 105);
                    break;
                }
            }

            int pillWidth = std::min(opt.rect.width() - 8, 56);
            int pillHeight = 16;
            int pillX = opt.rect.left() + (opt.rect.width() - pillWidth) / 2;
            int pillY = opt.rect.top() + (opt.rect.height() - pillHeight) / 2;
            QRect pillRect(pillX, pillY, pillWidth, pillHeight);

            painter->setBrush(pillBg);
            painter->setPen(pillBorder);
            painter->drawRoundedRect(pillRect, 3, 3);

            QFont badgeFont = opt.font;
            badgeFont.setPointSize(8);
            badgeFont.setBold(true);
            painter->setFont(badgeFont);
            painter->setPen(pillText);
            painter->drawText(pillRect, Qt::AlignCenter, opt.text);
        }
    } else if (col == Model::LogModel::ColMessage) {
        painter->setFont(opt.font);
        QRect textRect = opt.rect.adjusted(6, 0, -4, 0);

        QString text = opt.text;
        const QColor msgColor = m_isDark ? QColor(244, 244, 245) : QColor(15, 23, 42);

        // Check if there is search highlight to draw
        if (!m_searchQuery.isEmpty() && !text.isEmpty()) {
            Qt::CaseSensitivity cs = m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
            int matchIdx = -1;
            int matchLen = 0;

            if (m_isRegex) {
                QRegularExpression::PatternOptions popts = QRegularExpression::NoPatternOption;
                if (!m_caseSensitive) popts |= QRegularExpression::CaseInsensitiveOption;
                QRegularExpression rx(m_searchQuery, popts);
                auto m = rx.match(text);
                if (m.hasMatch()) {
                    matchIdx = m.capturedStart();
                    matchLen = m.capturedLength();
                }
            } else {
                matchIdx = text.indexOf(m_searchQuery, 0, cs);
                matchLen = m_searchQuery.length();
            }

            if (matchIdx >= 0 && matchLen > 0) {
                QFontMetrics fm(opt.font);
                QString before = text.left(matchIdx);
                QString matched = text.mid(matchIdx, matchLen);
                QString after = text.mid(matchIdx + matchLen);

                int x = textRect.left();
                int y = textRect.top() + (textRect.height() + fm.ascent() - fm.descent()) / 2;

                // Draw before
                painter->setPen(msgColor);
                painter->drawText(x, y, before);
                x += fm.horizontalAdvance(before);

                // Highlight box
                int matchWidth = fm.horizontalAdvance(matched);
                QRect hlBox(x - 1, textRect.top() + 2, matchWidth + 2, textRect.height() - 4);
                if (m_isDark) {
                    painter->fillRect(hlBox, QColor(133, 77, 14)); // Yellow 800
                    painter->setPen(QColor(254, 240, 138)); // Yellow 200
                } else {
                    painter->fillRect(hlBox, QColor(254, 240, 138)); // Yellow 200
                    painter->setPen(QColor(113, 63, 18)); // Yellow 900
                }
                painter->drawText(x, y, matched);
                x += matchWidth;

                // Draw after
                painter->setPen(msgColor);
                painter->drawText(x, y, after);

                painter->restore();
                return;
            }
        }

        painter->setPen(msgColor);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }

    painter->restore();
}

} // namespace Munin::UI
