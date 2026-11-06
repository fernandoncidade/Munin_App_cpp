#pragma once

#include "../engine/ParallelSearch.hpp"

#include <QWidget>

class QLineEdit;
class QPushButton;
class QToolButton;
class QCheckBox;
class QLabel;
class QProgressBar;

namespace Munin::UI {

class SearchBarWidget : public QWidget {
    Q_OBJECT

public:
    explicit SearchBarWidget(QWidget* parent = nullptr);

    void setMatchResults(int currentIndex, int totalMatches, double elapsedSeconds);
    void setSearching(bool searching);
    void setProgress(float progress);
    void setQuery(const QString& query);
    void retranslateUi();
    void applyTheme(bool isDark);

protected:
    void changeEvent(QEvent* event) override;

signals:
    void searchTriggered(const Engine::SearchOptions& options);
    void nextMatchTriggered();
    void prevMatchTriggered();
    void filterModeToggled(bool filterActive);
    void clearTriggered();

private slots:
    void onSearchSubmitted();
    void onOptionChanged();

private:
    [[nodiscard]] Engine::SearchOptions collectOptions() const;

    QLineEdit* m_searchEdit{nullptr};
    QPushButton* m_regexBtn{nullptr};
    QPushButton* m_caseBtn{nullptr};
    QPushButton* m_invertBtn{nullptr};
    QPushButton* m_filterBtn{nullptr};
    QToolButton* m_prevBtn{nullptr};
    QToolButton* m_nextBtn{nullptr};
    QPushButton* m_closeBtn{nullptr};
    QLabel* m_statusLabel{nullptr};
    QProgressBar* m_progressBar{nullptr};
};

} // namespace Munin::UI
