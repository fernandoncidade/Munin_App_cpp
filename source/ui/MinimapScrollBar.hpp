#pragma once

#include <QWidget>
#include <vector>

namespace Munin::Model {
class LogModel;
}

namespace Munin::UI {

class MinimapScrollBar : public QWidget {
    Q_OBJECT

public:
    explicit MinimapScrollBar(QWidget* parent = nullptr);

    void setModel(Model::LogModel* model);
    void setVisibleRange(int firstVisibleRow, int visibleRowCount);
    void setSearchMarkers(std::vector<uint64_t> lineIndices);
    void setErrorMarkers(std::vector<uint64_t> lineIndices);
    void clearMarkers();
    void setDarkTheme(bool isDark) {
        if (m_isDark != isDark) {
            m_isDark = isDark;
            update();
        }
    }

signals:
    void seekRequested(int row);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    int mapYToRow(int y) const;
    int mapRowToY(uint64_t row) const;

    Model::LogModel* m_model{nullptr};
    int m_firstVisibleRow{0};
    int m_visibleRowCount{40};

    std::vector<uint64_t> m_searchMarkers;
    std::vector<uint64_t> m_errorMarkers;

    bool m_isDragging{false};
    int m_hoverY{-1};
    bool m_isDark{true};
};

} // namespace Munin::UI
