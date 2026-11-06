#pragma once

#include "../engine/ChunkIndex.hpp"
#include "../engine/LiveTailWatcher.hpp"
#include "../engine/MemoryMappedFile.hpp"
#include "../engine/ParallelSearch.hpp"
#include "../language/GerenciadorTraducao.hpp"
#include "../model/LogModel.hpp"
#include "ThemeManager.hpp"

#include <QMainWindow>
#include <memory>

class QSplitter;
class QLabel;
class QPushButton;
class QTimer;
class QToolBar;
class QMenuBar;
class QMenu;
class QAction;
class QActionGroup;
class QShortcut;

namespace Munin::UI {

class LogView;
class MinimapScrollBar;
class TimeSliceWidget;
class JsonInspectorWidget;
class SearchBarWidget;
class StickyMenu;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void openLogFile(const QString& filePath);
    void retranslateUi();
    void applyThemeStyles(bool isDark);

protected:
    void changeEvent(QEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onActionOpen();
    void onActionLiveTailToggled(bool active);
    void onActionToggleSearch();
    void onActionToggleTimeline();
    void onActionToggleJsonInspector();
    void onActionGenerateLog();
    void onClearFilter();
    void onActionManual();
    void onActionSobre();
    void onMudarIdioma(const QString& codigoIdioma);

    // Engine callbacks connected via Qt events
    void onFileGrown(uint64_t oldSize, uint64_t newSize);
    void onSearchTriggered(const Engine::SearchOptions& options);
    void onSearchFinished(const std::vector<Engine::SearchMatch>& matches, double elapsed);
    void onTimeSliceClicked(uint64_t lineIndex);
    void onLineSelected(uint64_t realLine, const QString& fullLine);
    void onSeekRequested(int row);
    void updateSystemStats();

private:
    void setupMenuBar();
    void setupUi();
    void setupToolBar();
    void setupStatusBar();
    void triggerBackgroundAnalysis();
    void updateStatusSizeLabel();
    void updateStatusLinesLabel();

    // Translation Manager
    std::unique_ptr<source::language::GerenciadorTraducao> m_gerenciadorTraducao;

    // Menu Bar & Actions (Structured natively using StickyMenu like Linceu_Lighthouse_cpp)
    QMenuBar* m_menuBar{nullptr};

    StickyMenu* m_menuArquivo{nullptr};
    QAction* m_actionOpen{nullptr};
    QAction* m_actionGenerateLog{nullptr};
    QAction* m_actionExit{nullptr};

    StickyMenu* m_menuExibir{nullptr};
    QAction* m_actionToggleToolBar{nullptr};
    QAction* m_actionToggleSearch{nullptr};
    QAction* m_actionToggleTimeline{nullptr};
    QAction* m_actionToggleJsonInspector{nullptr};
    QAction* m_actionLiveTail{nullptr};
    QAction* m_actionClearFilter{nullptr};

    StickyMenu* m_menuConfig{nullptr};
    StickyMenu* m_menuIdiomas{nullptr};
    QActionGroup* m_langActionGroup{nullptr};
    QAction* m_actionPtBr{nullptr};
    QAction* m_actionEnUs{nullptr};

    StickyMenu* m_menuSobre{nullptr};
    QAction* m_actionManual{nullptr};
    QAction* m_actionSobre{nullptr};

    // Menu Shortcuts (Alt+A, Alt+E, Alt+C, Alt+B)
    QShortcut* m_shortcutArquivo{nullptr};
    QShortcut* m_shortcutExibir{nullptr};
    QShortcut* m_shortcutConfig{nullptr};
    QShortcut* m_shortcutSobre{nullptr};

    // Engine instances
    std::shared_ptr<Engine::MemoryMappedFile> m_mmap;
    std::shared_ptr<Engine::ChunkIndex> m_index;
    std::unique_ptr<Engine::LiveTailWatcher> m_liveTail;
    std::unique_ptr<Engine::ParallelSearch> m_searchEngine;

    // Model & UI Widgets
    Model::LogModel* m_logModel{nullptr};
    LogView* m_logView{nullptr};
    MinimapScrollBar* m_minimap{nullptr};
    TimeSliceWidget* m_timeSliceWidget{nullptr};
    JsonInspectorWidget* m_jsonInspector{nullptr};
    SearchBarWidget* m_searchBar{nullptr};

    QSplitter* m_mainSplitter{nullptr};
    QToolBar* m_toolBar{nullptr};
    QPushButton* m_openBtn{nullptr};
    QPushButton* m_liveTailBtn{nullptr};
    QPushButton* m_searchToggleBtn{nullptr};
    QPushButton* m_timelineToggleBtn{nullptr};
    QPushButton* m_jsonToggleBtn{nullptr};
    QPushButton* m_genBtn{nullptr};
    QPushButton* m_clearFilterBtn{nullptr};
    QWidget* m_filterBadgeWidget{nullptr};
    QLabel* m_filterBadgeLabel{nullptr};

    // Status Bar Labels
    QLabel* m_statusFileLabel{nullptr};
    QLabel* m_statusSizeLabel{nullptr};
    QLabel* m_statusLinesLabel{nullptr};
    QLabel* m_statusSpeedLabel{nullptr};
    QLabel* m_statusRamLabel{nullptr};
    QLabel* m_statusTailLabel{nullptr};

    QTimer* m_statsTimer{nullptr};

    // Search state
    std::vector<Engine::SearchMatch> m_activeMatches;
    int m_currentMatchIndex{-1};
    bool m_filterModeActive{false};
    QString m_openedFilePath;
    uint64_t m_currentDisplayedLines{0};
    bool m_isIndexing{false};
};

} // namespace Munin::UI
