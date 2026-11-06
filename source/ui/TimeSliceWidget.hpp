#pragma once

#include "../engine/TimeSliceScanner.hpp"

#include <QWidget>

namespace Munin::UI {

class TimeSliceWidget : public QWidget {
    Q_OBJECT

public:
    explicit TimeSliceWidget(QWidget* parent = nullptr);

    void setTimeSliceData(const Engine::TimeSliceResult& result);
    void clear();
    void retranslateUi() { update(); }
    void setDarkTheme(bool isDark) {
        if (m_isDark != isDark) {
            m_isDark = isDark;
            update();
        }
    }

signals:
    void timeSliceClicked(uint64_t targetLineIndex);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    int getBucketAtX(int x) const;

    Engine::TimeSliceResult m_data;
    uint64_t m_maxCount{1};
    int m_hoveredBucket{-1};
    bool m_isDark{true};
};

} // namespace Munin::UI
