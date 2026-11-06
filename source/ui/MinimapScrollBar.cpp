#include "MinimapScrollBar.hpp"
#include "ThemeManager.hpp"
#include "../model/LogModel.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

namespace Munin::UI {

MinimapScrollBar::MinimapScrollBar(QWidget* parent)
    : QWidget(parent)
    , m_isDark(ThemeManager::instance().isDark()) {
    setFixedWidth(28);
    setMouseTracking(true);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &MinimapScrollBar::setDarkTheme);
}

void MinimapScrollBar::setModel(Model::LogModel* model) {
    m_model = model;
    update();
}

void MinimapScrollBar::setVisibleRange(int firstVisibleRow, int visibleRowCount) {
    m_firstVisibleRow = firstVisibleRow;
    m_visibleRowCount = visibleRowCount;
    update();
}

void MinimapScrollBar::setSearchMarkers(std::vector<uint64_t> lineIndices) {
    m_searchMarkers = std::move(lineIndices);
    update();
}

void MinimapScrollBar::setErrorMarkers(std::vector<uint64_t> lineIndices) {
    m_errorMarkers = std::move(lineIndices);
    update();
}

void MinimapScrollBar::clearMarkers() {
    m_searchMarkers.clear();
    m_errorMarkers.clear();
    update();
}

int MinimapScrollBar::mapRowToY(uint64_t row) const {
    if (!m_model) return 0;
    int total = m_model->rowCount();
    if (total <= 1) return 0;

    double ratio = static_cast<double>(row) / static_cast<double>(total - 1);
    return static_cast<int>(ratio * (height() - 1));
}

int MinimapScrollBar::mapYToRow(int y) const {
    if (!m_model) return 0;
    int total = m_model->rowCount();
    if (total <= 1 || height() <= 1) return 0;

    y = std::clamp(y, 0, height() - 1);
    double ratio = static_cast<double>(y) / static_cast<double>(height() - 1);
    int targetRow = static_cast<int>(ratio * (total - 1));
    return std::clamp(targetRow, 0, total - 1);
}

void MinimapScrollBar::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Background
    QColor bgColor = m_isDark ? QColor(24, 24, 27) : QColor(248, 250, 252);
    QColor borderColor = m_isDark ? QColor(39, 39, 42) : QColor(226, 232, 240);
    painter.fillRect(rect(), bgColor);
    painter.setPen(borderColor);
    painter.drawLine(0, 0, 0, height());

    if (!m_model || m_model->rowCount() == 0) {
        return;
    }

    int w = width();

    // Draw Search Markers (Sky Cyan / Sky Blue)
    painter.setPen(m_isDark ? QColor(56, 189, 248) : QColor(2, 132, 199));
    for (uint64_t lineIdx : m_searchMarkers) {
        int y = mapRowToY(lineIdx);
        painter.drawLine(3, y, w - 3, y);
    }

    // Draw Error Markers (Red)
    painter.setPen(QColor(239, 68, 68));
    for (uint64_t lineIdx : m_errorMarkers) {
        int y = mapRowToY(lineIdx);
        painter.drawLine(2, y, w - 2, y);
    }

    // Draw Viewport Thumb
    int thumbTop = mapRowToY(m_firstVisibleRow);
    int thumbBottom = mapRowToY(m_firstVisibleRow + m_visibleRowCount);
    int thumbHeight = std::max(8, thumbBottom - thumbTop);

    QRect thumbRect(2, thumbTop, w - 4, thumbHeight);
    painter.fillRect(thumbRect, m_isDark ? QColor(63, 63, 70, 140) : QColor(148, 163, 184, 140));
    painter.setPen(m_isDark ? QColor(113, 113, 122, 180) : QColor(100, 116, 139, 180));
    painter.drawRect(thumbRect);

    // Hover indicator
    if (m_hoverY >= 0 && m_hoverY < height()) {
        painter.setPen(m_isDark ? QColor(250, 250, 250, 160) : QColor(15, 23, 42, 160));
        painter.drawLine(0, m_hoverY, w, m_hoverY);
    }
}

void MinimapScrollBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        int row = mapYToRow(event->pos().y());
        emit seekRequested(row);
    }
}

void MinimapScrollBar::mouseMoveEvent(QMouseEvent* event) {
    m_hoverY = event->pos().y();

    if (m_isDragging) {
        int row = mapYToRow(event->pos().y());
        emit seekRequested(row);
    } else if (m_model && m_model->rowCount() > 0) {
        int targetRow = mapYToRow(m_hoverY);
        uint64_t realLine = m_model->mapRowToRealLine(targetRow);
        QString linePreview = m_model->getLineString(realLine);
        if (linePreview.length() > 100) {
            linePreview = linePreview.left(97) + QStringLiteral("...");
        }
        QString tip = QCoreApplication::translate("InterfaceGrafica", "Linha %1").arg(realLine + 1) + QLatin1Char('\n') + linePreview;
        QToolTip::showText(event->globalPosition().toPoint(), tip, this);
    }

    update();
}

void MinimapScrollBar::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
    }
}

void MinimapScrollBar::leaveEvent(QEvent* event) {
    Q_UNUSED(event);
    m_hoverY = -1;
    m_isDragging = false;
    update();
}

} // namespace Munin::UI
