#include "TimeSliceWidget.hpp"
#include "ThemeManager.hpp"

#include <QCoreApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

namespace Munin::UI {

TimeSliceWidget::TimeSliceWidget(QWidget* parent)
    : QWidget(parent)
    , m_isDark(ThemeManager::instance().isDark()) {
    setFixedHeight(105);
    setMouseTracking(true);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &TimeSliceWidget::setDarkTheme);
}

void TimeSliceWidget::clear() {
    m_data = {};
    m_maxCount = 1;
    m_hoveredBucket = -1;
    update();
}

void TimeSliceWidget::setTimeSliceData(const Engine::TimeSliceResult& result) {
    m_data = result;
    m_maxCount = 1;
    m_hoveredBucket = -1;

    for (const auto& b : m_data.buckets) {
        if (b.count > m_maxCount) {
            m_maxCount = b.count;
        }
    }

    update();
}

int TimeSliceWidget::getBucketAtX(int x) const {
    if (!m_data.hasTimestamps || m_data.buckets.empty()) return -1;

    QRect chartRect = rect().adjusted(16, 26, -16, -18);
    if (!chartRect.contains(x, chartRect.center().y())) return -1;

    double ratio = static_cast<double>(x - chartRect.left()) / static_cast<double>(chartRect.width());
    int idx = static_cast<int>(ratio * m_data.buckets.size());
    return std::clamp(idx, 0, static_cast<int>(m_data.buckets.size()) - 1);
}

void TimeSliceWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Background
    QColor bgColor = m_isDark ? QColor(20, 20, 23) : QColor(248, 250, 252);
    QColor borderColor = m_isDark ? QColor(39, 39, 42) : QColor(226, 232, 240);
    painter.fillRect(rect(), bgColor);
    painter.setPen(borderColor);
    painter.drawLine(0, 0, width(), 0);
    painter.drawLine(0, height() - 1, width(), height() - 1);

    if (!m_data.hasTimestamps || m_data.buckets.empty()) {
        painter.setPen(m_isDark ? QColor(113, 113, 122) : QColor(100, 116, 139));
        QFont italicFont = font();
        italicFont.setItalic(true);
        painter.setFont(italicFont);
        painter.drawText(rect(), Qt::AlignCenter, QCoreApplication::translate("InterfaceGrafica", "Distribuição temporal (ISO-8601) será gerada para arquivos com carimbo de data/hora"));
        return;
    }

    // Header Title
    QFont titleFont = font();
    titleFont.setPointSize(8);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(m_isDark ? QColor(161, 161, 170) : QColor(51, 65, 85));
    painter.drawText(16, 16, QCoreApplication::translate("InterfaceGrafica", "TIMELINE DE VOLUME DE LOGS"));

    // Time Span label
    if (!m_data.buckets.empty()) {
        QString timeSpan = QStringLiteral("%1 ➔ %2")
            .arg(QString::fromStdString(m_data.buckets.front().labelStart))
            .arg(QString::fromStdString(m_data.buckets.back().labelStart));
        painter.setPen(m_isDark ? QColor(113, 113, 122) : QColor(100, 116, 139));
        painter.drawText(rect().adjusted(16, 4, -16, -4), Qt::AlignRight | Qt::AlignTop, timeSpan);
    }

    QRect chartRect = rect().adjusted(16, 26, -16, -18);
    size_t numBuckets = m_data.buckets.size();
    double barStep = static_cast<double>(chartRect.width()) / static_cast<double>(numBuckets);
    double barWidth = std::max(2.0, barStep - 1.5);

    for (size_t b = 0; b < numBuckets; ++b) {
        const auto& bucket = m_data.buckets[b];
        double barX = chartRect.left() + b * barStep;

        double countRatio = static_cast<double>(bucket.count) / static_cast<double>(m_maxCount);
        int barHeight = static_cast<int>(countRatio * chartRect.height());
        barHeight = std::max(bucket.count > 0 ? 3 : 1, barHeight);

        int barY = chartRect.bottom() - barHeight;
        QRect barRect(static_cast<int>(barX), barY, static_cast<int>(barWidth), barHeight);

        bool isHovered = (static_cast<int>(b) == m_hoveredBucket);

        // Gradient or Error colors
        if (bucket.errorCount > 0) {
            // Error spike
            painter.fillRect(barRect, isHovered ? QColor(248, 113, 113) : QColor(239, 68, 68));
        } else if (bucket.count > 0) {
            // Normal log volume
            painter.fillRect(barRect, isHovered ? (m_isDark ? QColor(96, 165, 250) : QColor(37, 99, 235))
                                                : (m_isDark ? QColor(59, 130, 246) : QColor(2, 132, 199)));
        } else {
            painter.fillRect(barRect, m_isDark ? QColor(39, 39, 42) : QColor(226, 232, 240));
        }

        if (isHovered) {
            painter.setPen(m_isDark ? QColor(255, 255, 255) : QColor(15, 23, 42));
            painter.drawRect(barRect.adjusted(-1, -1, 1, 0));
        }
    }

    // X-Axis baseline
    painter.setPen(m_isDark ? QColor(63, 63, 70) : QColor(203, 213, 225));
    painter.drawLine(chartRect.left(), chartRect.bottom() + 1, chartRect.right(), chartRect.bottom() + 1);
}

void TimeSliceWidget::mouseMoveEvent(QMouseEvent* event) {
    int bucketIdx = getBucketAtX(event->pos().x());
    if (bucketIdx != m_hoveredBucket) {
        m_hoveredBucket = bucketIdx;
        update();

        if (m_hoveredBucket >= 0 && m_hoveredBucket < static_cast<int>(m_data.buckets.size())) {
            const auto& b = m_data.buckets[m_hoveredBucket];
            QStringList lines;
            lines << QCoreApplication::translate("InterfaceGrafica", "Período: %1").arg(QString::fromStdString(b.labelStart));
            lines << QCoreApplication::translate("InterfaceGrafica", "Volume: %1 eventos").arg(b.count);
            lines << QCoreApplication::translate("InterfaceGrafica", "Erros: %1").arg(b.errorCount);
            lines << QCoreApplication::translate("InterfaceGrafica", "Clique para rolar até este momento");

            QToolTip::showText(event->globalPosition().toPoint(), lines.join(QLatin1Char('\n')), this);
        } else {
            QToolTip::hideText();
        }
    }
}

void TimeSliceWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        int bucketIdx = getBucketAtX(event->pos().x());
        if (bucketIdx >= 0 && bucketIdx < static_cast<int>(m_data.buckets.size())) {
            const auto& b = m_data.buckets[bucketIdx];
            emit timeSliceClicked(b.firstLineIndex);
        }
    }
}

void TimeSliceWidget::leaveEvent(QEvent* event) {
    Q_UNUSED(event);
    m_hoveredBucket = -1;
    QToolTip::hideText();
    update();
}

void TimeSliceWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        update();
    }
    QWidget::changeEvent(event);
}

} // namespace Munin::UI
