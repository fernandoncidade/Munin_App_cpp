#pragma once

#include <QTableView>

namespace Munin::Model {
class LogModel;
}

namespace Munin::UI {

class LogItemDelegate;
class MinimapScrollBar;

class LogView : public QTableView {
    Q_OBJECT

public:
    explicit LogView(QWidget* parent = nullptr);

    void setLogModel(Model::LogModel* model);
    void setMinimap(MinimapScrollBar* minimap);
    [[nodiscard]] LogItemDelegate* logDelegate() const noexcept { return m_delegate; }

    void scrollToRow(int row);
    void applyTheme(bool isDark);

signals:
    void filterByStringRequested(const QString& target);
    void searchStringRequested(const QString& target);
    void inspectJsonRequested(const QString& fullLine);
    void lineSelected(uint64_t realLine, const QString& fullLine);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;

private slots:
    void onScrollChanged(int value);

private:
    void updateMinimapRange();
    static QString extractThreadOrIp(const QString& line);

    Model::LogModel* m_model{nullptr};
    LogItemDelegate* m_delegate{nullptr};
    MinimapScrollBar* m_minimap{nullptr};
};

} // namespace Munin::UI
