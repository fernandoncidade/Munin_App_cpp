#include "MainWindow.hpp"
#include "ButtonUtils.hpp"
#include "GerenciadorDialogos.hpp"
#include "JsonInspectorWidget.hpp"
#include "LogGeneratorDialog.hpp"
#include "LogItemDelegate.hpp"
#include "LogView.hpp"
#include "Manual.hpp"
#include "MinimapScrollBar.hpp"
#include "SearchBarWidget.hpp"
#include "SobreDialog.hpp"
#include "StickyMenu.hpp"
#include "ThemeManager.hpp"
#include "TimeSliceWidget.hpp"
#include "Win32DwmHelper.hpp"
#include "../engine/TimeSliceScanner.hpp"
#include "../utils/ApplicationPathUtils.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolTip>
#include <QVBoxLayout>

#if defined(Q_OS_WIN)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace Munin::UI {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_gerenciadorTraducao(std::make_unique<source::language::GerenciadorTraducao>())
    , m_mmap(std::make_shared<Engine::MemoryMappedFile>())
    , m_index(std::make_shared<Engine::ChunkIndex>())
    , m_liveTail(std::make_unique<Engine::LiveTailWatcher>())
    , m_searchEngine(std::make_unique<Engine::ParallelSearch>())
    , m_logModel(new Model::LogModel(this))
    , m_statsTimer(new QTimer(this)) {

    // Configure main window properties
    setAcceptDrops(true);
    setWindowIcon(get_app_icon());
    resize(1360, 840);
    setMinimumSize(800, 500);

    GerenciadorDialogos::instance().inicializar(this);

    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();

    // Connect to dynamic theme manager
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &MainWindow::applyThemeStyles);
    applyThemeStyles(ThemeManager::instance().isDark());

    // Connect translation manager
    if (m_gerenciadorTraducao) {
        connect(m_gerenciadorTraducao.get(), &source::language::GerenciadorTraducao::idioma_alterado,
                this, [this](const QString& idioma) {
            retranslateUi();
            GerenciadorDialogos::instance().atualizar_traducoes(idioma);
        });
    }

    retranslateUi();

    // Periodic system stats refresh (RAM & Indexer)
    connect(m_statsTimer, &QTimer::timeout, this, &MainWindow::updateSystemStats);
    m_statsTimer->start(1000);
}

MainWindow::~MainWindow() {
    GerenciadorDialogos::instance().fechar_todos();
    if (m_liveTail) m_liveTail->stop();
    if (m_searchEngine) m_searchEngine->cancel();
    if (m_index) m_index->stopIndexing();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    GerenciadorDialogos::instance().fechar_todos();
    QMainWindow::closeEvent(event);
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    Win32DwmHelper::applyTitleBarTheme(this, ThemeManager::instance().isDark());
}

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
#if defined(Q_OS_WIN)
    MSG* msg = static_cast<MSG*>(message);
    if (msg && msg->message == WM_SETTINGCHANGE) {
        if (!msg->lParam) {
            ThemeManager::instance().checkSystemThemePoll();
        } else {
            LPCWSTR paramStr = reinterpret_cast<LPCWSTR>(msg->lParam);
            if (wcscmp(paramStr, L"ImmersiveColorSet") == 0 ||
                wcscmp(paramStr, L"WindowsThemeChanged") == 0) {
                ThemeManager::instance().checkSystemThemePoll();
            }
        }
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::setupUi() {
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Search bar (at top, toggleable)
    m_searchBar = new SearchBarWidget(this);
    m_searchBar->setVisible(false);
    rootLayout->addWidget(m_searchBar);

    connect(m_searchBar, &SearchBarWidget::searchTriggered, this, &MainWindow::onSearchTriggered);
    connect(m_searchBar, &SearchBarWidget::clearTriggered, this, &MainWindow::onClearFilter);
    connect(m_searchBar, &SearchBarWidget::nextMatchTriggered, this, [this]() {
        if (m_activeMatches.empty()) return;
        m_currentMatchIndex = (m_currentMatchIndex + 1) % static_cast<int>(m_activeMatches.size());
        m_logView->scrollToRow(static_cast<int>(m_activeMatches[m_currentMatchIndex].lineIndex));
        m_searchBar->setMatchResults(m_currentMatchIndex, static_cast<int>(m_activeMatches.size()), 0.0);
    });
    connect(m_searchBar, &SearchBarWidget::prevMatchTriggered, this, [this]() {
        if (m_activeMatches.empty()) return;
        m_currentMatchIndex = (m_currentMatchIndex - 1 + static_cast<int>(m_activeMatches.size())) % static_cast<int>(m_activeMatches.size());
        m_logView->scrollToRow(static_cast<int>(m_activeMatches[m_currentMatchIndex].lineIndex));
        m_searchBar->setMatchResults(m_currentMatchIndex, static_cast<int>(m_activeMatches.size()), 0.0);
    });
    connect(m_searchBar, &SearchBarWidget::filterModeToggled, this, [this](bool active) {
        m_filterModeActive = active;
        if (active && !m_activeMatches.empty()) {
            std::vector<uint64_t> lines;
            lines.reserve(m_activeMatches.size());
            for (const auto& m : m_activeMatches) {
                lines.push_back(m.lineIndex);
            }
            m_logModel->setFilteredLines(std::move(lines), QStringLiteral("Busca"));
        } else {
            m_logModel->clearFilter();
        }
    });

    // TimeSlice Timeline Chart (toggleable)
    m_timeSliceWidget = new TimeSliceWidget(this);
    m_timeSliceWidget->setVisible(false);
    connect(m_timeSliceWidget, &TimeSliceWidget::timeSliceClicked, this, &MainWindow::onTimeSliceClicked);
    rootLayout->addWidget(m_timeSliceWidget);

    // Main Splitter: Left (LogView + Minimap), Right (JSON Inspector)
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setHandleWidth(2);

    auto* leftContainer = new QWidget(m_mainSplitter);
    auto* leftLayout = new QHBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    // Table view for log records
    m_logView = new LogView(leftContainer);
    leftLayout->addWidget(m_logView, 1);

    // Minimap scrollbar
    m_minimap = new MinimapScrollBar(leftContainer);
    m_minimap->setModel(m_logModel);
    m_logView->setMinimap(m_minimap);
    connect(m_minimap, &MinimapScrollBar::seekRequested, this, &MainWindow::onSeekRequested);
    leftLayout->addWidget(m_minimap);

    m_mainSplitter->addWidget(leftContainer);

    // JSON and record details inspector
    m_jsonInspector = new JsonInspectorWidget(m_mainSplitter);
    m_mainSplitter->addWidget(m_jsonInspector);

    m_mainSplitter->setStretchFactor(0, 7);
    m_mainSplitter->setStretchFactor(1, 3);
    rootLayout->addWidget(m_mainSplitter, 1);

    // Connect LogView selection to Inspector
    connect(m_logView, &LogView::lineSelected, this, &MainWindow::onLineSelected);
    connect(m_logView, &LogView::inspectJsonRequested, this, [this](const QString& line) {
        m_jsonInspector->inspectLine(0, 0, line);
        if (!m_jsonInspector->isVisible()) {
            m_jsonInspector->setVisible(true);
            if (m_actionToggleJsonInspector) m_actionToggleJsonInspector->setChecked(true);
            if (m_jsonToggleBtn) m_jsonToggleBtn->setChecked(true);
        }
    });
    connect(m_logView, &LogView::searchStringRequested, this, [this](const QString& target) {
        m_searchBar->setVisible(true);
        if (m_actionToggleSearch) m_actionToggleSearch->setChecked(true);
        if (m_searchToggleBtn) m_searchToggleBtn->setChecked(true);
        m_searchBar->setQuery(target);
    });
    connect(m_logView, &LogView::filterByStringRequested, this, [this](const QString& target) {
        m_searchBar->setVisible(true);
        if (m_actionToggleSearch) m_actionToggleSearch->setChecked(true);
        if (m_searchToggleBtn) m_searchToggleBtn->setChecked(true);
        m_searchBar->setQuery(target);
    });
}

void MainWindow::setupMenuBar() {
    m_menuBar = new QMenuBar(this);
    m_menuBar->setNativeMenuBar(false);
    m_menuBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setMenuBar(m_menuBar);

    // Menu: Arquivo
    m_menuArquivo = create_menu(QCoreApplication::translate("InterfaceGrafica", "&Arquivo"), m_menuBar);
    m_menuBar->addMenu(m_menuArquivo);

    m_actionOpen = m_menuArquivo->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton),
                                            QCoreApplication::translate("InterfaceGrafica", "&Abrir Arquivo..."));
    m_actionOpen->setShortcut(QKeySequence::Open);
    connect(m_actionOpen, &QAction::triggered, this, &MainWindow::onActionOpen);

    m_actionGenerateLog = m_menuArquivo->addAction(style()->standardIcon(QStyle::SP_FileIcon),
                                                   QCoreApplication::translate("InterfaceGrafica", "&Gerar Log Sintético..."));
    m_actionGenerateLog->setShortcut(QKeySequence(QStringLiteral("Ctrl+G")));
    connect(m_actionGenerateLog, &QAction::triggered, this, &MainWindow::onActionGenerateLog);

    m_menuArquivo->addSeparator();

    m_actionExit = m_menuArquivo->addAction(style()->standardIcon(QStyle::SP_DialogCloseButton),
                                            QCoreApplication::translate("InterfaceGrafica", "&Sair"));
    m_actionExit->setShortcut(QKeySequence::Quit);
    connect(m_actionExit, &QAction::triggered, this, &QWidget::close);

    // Menu: Exibir
    m_menuExibir = create_menu(QCoreApplication::translate("InterfaceGrafica", "&Exibir"), m_menuBar);
    m_menuBar->addMenu(m_menuExibir);

    m_actionToggleToolBar = m_menuExibir->addAction(QCoreApplication::translate("InterfaceGrafica", "Barra de Ferramentas"));
    m_actionToggleToolBar->setCheckable(true);
    m_actionToggleToolBar->setChecked(true);
    connect(m_actionToggleToolBar, &QAction::toggled, this, [this](bool visible) {
        if (m_toolBar) m_toolBar->setVisible(visible);
    });

    m_menuExibir->addSeparator();

    m_actionToggleSearch = m_menuExibir->addAction(style()->standardIcon(QStyle::SP_FileDialogContentsView),
                                                   QCoreApplication::translate("InterfaceGrafica", "Barra de &Busca"));
    m_actionToggleSearch->setShortcut(QKeySequence::Find);
    m_actionToggleSearch->setCheckable(true);
    connect(m_actionToggleSearch, &QAction::triggered, this, &MainWindow::onActionToggleSearch);

    m_actionToggleTimeline = m_menuExibir->addAction(style()->standardIcon(QStyle::SP_FileDialogListView),
                                                     QCoreApplication::translate("InterfaceGrafica", "&Timeline de Distribuição"));
    m_actionToggleTimeline->setShortcut(QKeySequence(QStringLiteral("Ctrl+T")));
    m_actionToggleTimeline->setCheckable(true);
    connect(m_actionToggleTimeline, &QAction::triggered, this, &MainWindow::onActionToggleTimeline);

    m_actionToggleJsonInspector = m_menuExibir->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView),
                                                          QCoreApplication::translate("InterfaceGrafica", "Inspetor &JSON"));
    m_actionToggleJsonInspector->setShortcut(QKeySequence(QStringLiteral("Ctrl+J")));
    m_actionToggleJsonInspector->setCheckable(true);
    m_actionToggleJsonInspector->setChecked(true);
    connect(m_actionToggleJsonInspector, &QAction::triggered, this, &MainWindow::onActionToggleJsonInspector);

    m_actionLiveTail = m_menuExibir->addAction(style()->standardIcon(QStyle::SP_BrowserReload),
                                               QCoreApplication::translate("InterfaceGrafica", "&Live Tail"));
    m_actionLiveTail->setShortcut(QKeySequence(QStringLiteral("Ctrl+L")));
    m_actionLiveTail->setCheckable(true);
    connect(m_actionLiveTail, &QAction::toggled, this, [this](bool checked) {
        if (m_liveTailBtn && m_liveTailBtn->isChecked() != checked) {
            m_liveTailBtn->setChecked(checked);
        }
    });

    m_menuExibir->addSeparator();

    m_actionClearFilter = m_menuExibir->addAction(style()->standardIcon(QStyle::SP_DialogResetButton),
                                                  QCoreApplication::translate("InterfaceGrafica", "&Limpar Filtros"));
    m_actionClearFilter->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+Del")));
    connect(m_actionClearFilter, &QAction::triggered, this, &MainWindow::onClearFilter);

    // Menu: Configurações
    m_menuConfig = create_menu(QCoreApplication::translate("InterfaceGrafica", "&Configurações"), m_menuBar);
    m_menuBar->addMenu(m_menuConfig);

    // Submenu: Idiomas
    m_menuIdiomas = create_menu(QCoreApplication::translate("InterfaceGrafica", "&Idiomas"), m_menuConfig);
    add_submenu(m_menuConfig, m_menuIdiomas);

    m_langActionGroup = new QActionGroup(this);
    m_langActionGroup->setExclusive(true);

    m_actionPtBr = m_menuIdiomas->addAction(QCoreApplication::translate("InterfaceGrafica", "Português (Brasil)"));
    m_actionPtBr->setCheckable(true);
    m_langActionGroup->addAction(m_actionPtBr);
    connect(m_actionPtBr, &QAction::triggered, this, [this]() {
        onMudarIdioma(QStringLiteral("pt_BR"));
    });

    m_actionEnUs = m_menuIdiomas->addAction(QCoreApplication::translate("InterfaceGrafica", "English (United States)"));
    m_actionEnUs->setCheckable(true);
    m_langActionGroup->addAction(m_actionEnUs);
    connect(m_actionEnUs, &QAction::triggered, this, [this]() {
        onMudarIdioma(QStringLiteral("en_US"));
    });

    QString curLang = m_gerenciadorTraducao ? m_gerenciadorTraducao->obter_idioma_atual() : QStringLiteral("pt_BR");
    if (curLang.startsWith(QStringLiteral("en"), Qt::CaseInsensitive)) {
        m_actionEnUs->setChecked(true);
    } else {
        m_actionPtBr->setChecked(true);
    }

    // Menu: Sobre
    m_menuSobre = create_menu(QCoreApplication::translate("InterfaceGrafica", "So&bre"), m_menuBar);
    m_menuBar->addMenu(m_menuSobre);

    m_actionManual = m_menuSobre->addAction(style()->standardIcon(QStyle::SP_DialogHelpButton),
                                            QCoreApplication::translate("InterfaceGrafica", "&Manual de Instruções"));
    m_actionManual->setShortcut(QKeySequence::HelpContents);
    connect(m_actionManual, &QAction::triggered, this, &MainWindow::onActionManual);

    m_menuSobre->addSeparator();

    m_actionSobre = m_menuSobre->addAction(style()->standardIcon(QStyle::SP_MessageBoxInformation),
                                           QCoreApplication::translate("InterfaceGrafica", "So&bre"));
    connect(m_actionSobre, &QAction::triggered, this, &MainWindow::onActionSobre);

    // Atalhos nativos de menu do estilo Linceu_Lighthouse_cpp
    delete m_shortcutArquivo;
    delete m_shortcutExibir;
    delete m_shortcutConfig;
    delete m_shortcutSobre;

    m_shortcutArquivo = new QShortcut(QKeySequence(QStringLiteral("Alt+A")), this);
    m_shortcutArquivo->setContext(Qt::ApplicationShortcut);
    connect(m_shortcutArquivo, &QShortcut::activated, this, [this]() {
        popup_menu(m_menuArquivo, m_menuBar);
    });

    m_shortcutExibir = new QShortcut(QKeySequence(QStringLiteral("Alt+E")), this);
    m_shortcutExibir->setContext(Qt::ApplicationShortcut);
    connect(m_shortcutExibir, &QShortcut::activated, this, [this]() {
        popup_menu(m_menuExibir, m_menuBar);
    });

    m_shortcutConfig = new QShortcut(QKeySequence(QStringLiteral("Alt+C")), this);
    m_shortcutConfig->setContext(Qt::ApplicationShortcut);
    connect(m_shortcutConfig, &QShortcut::activated, this, [this]() {
        popup_menu(m_menuConfig, m_menuBar);
    });

    m_shortcutSobre = new QShortcut(QKeySequence(QStringLiteral("Alt+B")), this);
    m_shortcutSobre->setContext(Qt::ApplicationShortcut);
    connect(m_shortcutSobre, &QShortcut::activated, this, [this]() {
        popup_menu(m_menuSobre, m_menuBar);
    });
}

void MainWindow::setupToolBar() {
    m_toolBar = addToolBar(QCoreApplication::translate("InterfaceGrafica", "Barra de Ferramentas Principal"));
    m_toolBar->setMovable(false);
    m_toolBar->setObjectName(QStringLiteral("mainToolBar"));

    // Open button
    m_openBtn = new QPushButton(this);
    m_openBtn->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    m_openBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Abrir Arquivo"));
    configure_license_action_button(m_openBtn, 90);
    connect(m_openBtn, &QPushButton::clicked, this, &MainWindow::onActionOpen);
    m_toolBar->addWidget(m_openBtn);

    m_toolBar->addSeparator();

    // Live Tail toggle button
    m_liveTailBtn = new QPushButton(this);
    m_liveTailBtn->setObjectName(QStringLiteral("liveTailBtn"));
    m_liveTailBtn->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    m_liveTailBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Live Tail: OFF"));
    m_liveTailBtn->setCheckable(true);
    configure_license_action_button(m_liveTailBtn, 95);
    connect(m_liveTailBtn, &QPushButton::toggled, this, &MainWindow::onActionLiveTailToggled);
    m_toolBar->addWidget(m_liveTailBtn);

    // Search toggle button
    m_searchToggleBtn = new QPushButton(this);
    m_searchToggleBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
    m_searchToggleBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Buscar"));
    m_searchToggleBtn->setCheckable(true);
    configure_license_action_button(m_searchToggleBtn, 75);
    connect(m_searchToggleBtn, &QPushButton::clicked, this, &MainWindow::onActionToggleSearch);
    m_toolBar->addWidget(m_searchToggleBtn);

    // Timeline toggle button
    m_timelineToggleBtn = new QPushButton(this);
    m_timelineToggleBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
    m_timelineToggleBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Timeline"));
    m_timelineToggleBtn->setCheckable(true);
    configure_license_action_button(m_timelineToggleBtn, 75);
    connect(m_timelineToggleBtn, &QPushButton::clicked, this, &MainWindow::onActionToggleTimeline);
    m_toolBar->addWidget(m_timelineToggleBtn);

    // JSON Inspector toggle button
    m_jsonToggleBtn = new QPushButton(this);
    m_jsonToggleBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    m_jsonToggleBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Inspetor JSON"));
    m_jsonToggleBtn->setCheckable(true);
    m_jsonToggleBtn->setChecked(true);
    configure_license_action_button(m_jsonToggleBtn, 95);
    connect(m_jsonToggleBtn, &QPushButton::clicked, this, &MainWindow::onActionToggleJsonInspector);
    m_toolBar->addWidget(m_jsonToggleBtn);

    m_toolBar->addSeparator();

    // Generate Synthetic Log button
    m_genBtn = new QPushButton(this);
    m_genBtn->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    m_genBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Gerador"));
    configure_license_action_button(m_genBtn, 75);
    connect(m_genBtn, &QPushButton::clicked, this, &MainWindow::onActionGenerateLog);
    m_toolBar->addWidget(m_genBtn);

    // Active filter badge widget
    m_filterBadgeWidget = new QWidget(this);
    auto* badgeLayout = new QHBoxLayout(m_filterBadgeWidget);
    badgeLayout->setContentsMargins(6, 0, 6, 0);
    badgeLayout->setSpacing(6);

    m_filterBadgeLabel = new QLabel(this);
    QPalette badgePal = m_filterBadgeLabel->palette();
    badgePal.setColor(QPalette::Window, QColor(2, 132, 199));
    badgePal.setColor(QPalette::WindowText, Qt::white);
    m_filterBadgeLabel->setAutoFillBackground(true);
    m_filterBadgeLabel->setPalette(badgePal);
    QFont badgeFont = m_filterBadgeLabel->font();
    badgeFont.setPointSize(8);
    badgeFont.setBold(true);
    m_filterBadgeLabel->setFont(badgeFont);
    m_filterBadgeLabel->setContentsMargins(6, 2, 6, 2);
    badgeLayout->addWidget(m_filterBadgeLabel);

    m_clearFilterBtn = new QPushButton(m_filterBadgeWidget);
    m_clearFilterBtn->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    configure_license_action_button(m_clearFilterBtn, 24);
    connect(m_clearFilterBtn, &QPushButton::clicked, this, &MainWindow::onClearFilter);
    badgeLayout->addWidget(m_clearFilterBtn);

    m_filterBadgeWidget->setVisible(false);
    m_toolBar->addWidget(m_filterBadgeWidget);

    connect(m_logModel, &Model::LogModel::filterChanged, this, [this](bool isFiltered, int count, const QString& desc) {
        if (isFiltered) {
            QString descLabel = desc.isEmpty() ? QCoreApplication::translate("InterfaceGrafica", "Ativo") : desc;
            m_filterBadgeLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Filtro: %1 (%2 linhas)")
                                            .arg(descLabel)
                                            .arg(count));
            m_filterBadgeWidget->setVisible(true);
        } else {
            m_filterBadgeWidget->setVisible(false);
        }
    });
}

void MainWindow::setupStatusBar() {
    auto* sBar = statusBar();

    m_statusFileLabel = new QLabel(QCoreApplication::translate("InterfaceGrafica", "Nenhum arquivo aberto"), this);
    sBar->addWidget(m_statusFileLabel, 1);

    m_statusSizeLabel = new QLabel(this);
    sBar->addPermanentWidget(m_statusSizeLabel);

    m_statusLinesLabel = new QLabel(this);
    sBar->addPermanentWidget(m_statusLinesLabel);

    m_statusSpeedLabel = new QLabel(QStringLiteral("Status: ") + QCoreApplication::translate("InterfaceGrafica", "Pronto"), this);
    sBar->addPermanentWidget(m_statusSpeedLabel);

    m_statusRamLabel = new QLabel(QStringLiteral("RAM: --"), this);
    sBar->addPermanentWidget(m_statusRamLabel);

    m_statusTailLabel = new QLabel(QCoreApplication::translate("InterfaceGrafica", "Tail: Inativo"), this);
    sBar->addPermanentWidget(m_statusTailLabel);

    updateStatusSizeLabel();
    updateStatusLinesLabel();
}

void MainWindow::updateStatusSizeLabel() {
    if (!m_statusSizeLabel) return;

    if (m_openedFilePath.isEmpty() || !m_mmap || !m_mmap->isOpen()) {
        m_statusSizeLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Tamanho: %1").arg(QStringLiteral("0 B")));
        return;
    }

    uint64_t fileSize = m_mmap->size();
    double sizeMB = static_cast<double>(fileSize) / (1024.0 * 1024.0);
    double sizeGB = sizeMB / 1024.0;

    QString sizeStr;
    if (sizeGB >= 1.0) {
        sizeStr = QStringLiteral("%1 GB").arg(sizeGB, 0, 'f', 2);
    } else if (sizeMB >= 1.0) {
        sizeStr = QStringLiteral("%1 MB").arg(sizeMB, 0, 'f', 1);
    } else if (fileSize >= 1024) {
        double sizeKB = static_cast<double>(fileSize) / 1024.0;
        sizeStr = QStringLiteral("%1 KB").arg(sizeKB, 0, 'f', 1);
    } else {
        sizeStr = QStringLiteral("%1 B").arg(fileSize);
    }

    m_statusSizeLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Tamanho: %1").arg(sizeStr));
}

void MainWindow::updateStatusLinesLabel() {
    if (!m_statusLinesLabel) return;

    if (m_openedFilePath.isEmpty() || !m_mmap || !m_mmap->isOpen()) {
        m_statusLinesLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Linhas: %1").arg(0));
        return;
    }

    if (m_isIndexing && m_currentDisplayedLines == 0) {
        m_statusLinesLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Linhas: %1")
                                       .arg(QCoreApplication::translate("InterfaceGrafica", "Indexando...")));
        return;
    }

    m_statusLinesLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Linhas: %1").arg(m_currentDisplayedLines));
}

void MainWindow::applyThemeStyles(bool isDark) {
    if (m_menuBar) {
        m_menuBar->setStyleSheet(QString());
    }
    if (m_toolBar) {
        m_toolBar->setStyleSheet(QString());
    }
    if (statusBar()) {
        statusBar()->setStyleSheet(QString());
    }
    if (m_mainSplitter) {
        m_mainSplitter->setStyleSheet(QString());
    }

    // Propagate to sub-widgets
    if (m_logView) m_logView->applyTheme(isDark);
    if (m_searchBar) m_searchBar->applyTheme(isDark);
    if (m_jsonInspector) m_jsonInspector->applyTheme(isDark);
    if (m_timeSliceWidget) m_timeSliceWidget->setDarkTheme(isDark);
    if (m_minimap) m_minimap->setDarkTheme(isDark);

    // Apply Win32 title bar theme
    Win32DwmHelper::applyTitleBarTheme(this, isDark);

    update();
}

void MainWindow::onActionManual() {
    QString idioma = m_gerenciadorTraducao ? m_gerenciadorTraducao->obter_idioma_atual() : QStringLiteral("pt_BR");
    GerenciadorDialogos::instance().exibir_manual(nullptr, idioma);
}

void MainWindow::onActionSobre() {
    QString idioma = m_gerenciadorTraducao ? m_gerenciadorTraducao->obter_idioma_atual() : QStringLiteral("pt_BR");
    GerenciadorDialogos::instance().exibir_sobre(nullptr, idioma);
}

void MainWindow::onMudarIdioma(const QString& codigoIdioma) {
    if (m_gerenciadorTraducao) {
        m_gerenciadorTraducao->definir_idioma(codigoIdioma);
    }
    retranslateUi();
    GerenciadorDialogos::instance().atualizar_traducoes(codigoIdioma);
}

void MainWindow::retranslateUi() {
    QToolTip::hideText();

    const QString idioma = m_gerenciadorTraducao ? m_gerenciadorTraducao->obter_idioma_atual() : QStringLiteral("pt_BR");
    const bool isPt = !idioma.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);

    setWindowTitle(isPt ? QStringLiteral("Munin App - Leitor de Logs Massivos")
                        : QStringLiteral("Munin App - Massive Log Viewer"));

    // Menus
    if (m_menuArquivo) {
        m_menuArquivo->setTitle(QCoreApplication::translate("InterfaceGrafica", "&Arquivo"));
    }
    if (m_actionOpen) {
        m_actionOpen->setText(QCoreApplication::translate("InterfaceGrafica", "&Abrir Arquivo..."));
        m_actionOpen->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Abrir Arquivo de Log"));
    }
    if (m_actionGenerateLog) {
        m_actionGenerateLog->setText(QCoreApplication::translate("InterfaceGrafica", "&Gerar Log Sintético..."));
        m_actionGenerateLog->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Gerar Log Sintético"));
    }
    if (m_actionExit) {
        m_actionExit->setText(QCoreApplication::translate("InterfaceGrafica", "&Sair"));
        m_actionExit->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Sair do Aplicativo"));
    }

    if (m_menuExibir) {
        m_menuExibir->setTitle(QCoreApplication::translate("InterfaceGrafica", "&Exibir"));
    }
    if (m_actionToggleToolBar) {
        m_actionToggleToolBar->setText(QCoreApplication::translate("InterfaceGrafica", "Barra de Ferramentas"));
        m_actionToggleToolBar->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Exibir ou ocultar a barra de ferramentas"));
    }
    if (m_actionToggleSearch) {
        m_actionToggleSearch->setText(QCoreApplication::translate("InterfaceGrafica", "Barra de &Busca"));
        m_actionToggleSearch->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Exibir ou ocultar a barra de busca"));
    }
    if (m_actionToggleTimeline) {
        m_actionToggleTimeline->setText(QCoreApplication::translate("InterfaceGrafica", "&Timeline de Distribuição"));
        m_actionToggleTimeline->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Exibir ou ocultar a timeline de distribuição"));
    }
    if (m_actionToggleJsonInspector) {
        m_actionToggleJsonInspector->setText(QCoreApplication::translate("InterfaceGrafica", "Inspetor &JSON"));
        m_actionToggleJsonInspector->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Exibir ou ocultar o inspetor JSON"));
    }
    if (m_actionLiveTail) {
        m_actionLiveTail->setText(QCoreApplication::translate("InterfaceGrafica", "&Live Tail"));
        m_actionLiveTail->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Modo Live Tail"));
    }
    if (m_actionClearFilter) {
        m_actionClearFilter->setText(QCoreApplication::translate("InterfaceGrafica", "&Limpar Filtros"));
        m_actionClearFilter->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Remover Filtro Ativo"));
    }

    if (m_menuConfig) {
        m_menuConfig->setTitle(QCoreApplication::translate("InterfaceGrafica", "&Configurações"));
    }
    if (m_menuIdiomas) {
        m_menuIdiomas->setTitle(QCoreApplication::translate("InterfaceGrafica", "&Idiomas"));
    }
    if (m_actionPtBr) {
        m_actionPtBr->setText(QCoreApplication::translate("InterfaceGrafica", "Português (Brasil)"));
        m_actionPtBr->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Português (Brasil)"));
        m_actionPtBr->setChecked(isPt);
    }
    if (m_actionEnUs) {
        m_actionEnUs->setText(QCoreApplication::translate("InterfaceGrafica", "English (United States)"));
        m_actionEnUs->setToolTip(QCoreApplication::translate("InterfaceGrafica", "English (United States)"));
        m_actionEnUs->setChecked(!isPt);
    }

    if (m_menuSobre) {
        m_menuSobre->setTitle(QCoreApplication::translate("InterfaceGrafica", "So&bre"));
    }
    if (m_actionManual) {
        m_actionManual->setText(QCoreApplication::translate("InterfaceGrafica", "&Manual de Instruções"));
        m_actionManual->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Manual de Instruções"));
    }
    if (m_actionSobre) {
        m_actionSobre->setText(QCoreApplication::translate("InterfaceGrafica", "So&bre"));
        m_actionSobre->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Sobre"));
    }

    // Toolbar buttons with multi-line informative tooltips and accessibility
    if (m_openBtn) {
        m_openBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Abrir Arquivo"));
        const QStringList openTooltip = {
            QCoreApplication::translate("InterfaceGrafica", "Abrir Arquivo de Log"),
            QCoreApplication::translate("InterfaceGrafica", "Carrega um arquivo de log local com mapeamento de memória instantâneo.")
        };
        m_openBtn->setToolTip(openTooltip.join(QLatin1Char('\n')));
        m_openBtn->setAccessibleName(m_openBtn->toolTip());
    }
    if (m_liveTailBtn) {
        bool active = m_liveTailBtn->isChecked();
        m_liveTailBtn->setText(active ? QCoreApplication::translate("InterfaceGrafica", "Live Tail: ON")
                                      : QCoreApplication::translate("InterfaceGrafica", "Live Tail: OFF"));
        const QStringList tailTooltip = {
            QCoreApplication::translate("InterfaceGrafica", "Modo Live Tail"),
            QCoreApplication::translate("InterfaceGrafica", "Monitora alterações e novas linhas no arquivo em tempo real (Win32 ReadDirectoryChangesW).")
        };
        m_liveTailBtn->setToolTip(tailTooltip.join(QLatin1Char('\n')));
        m_liveTailBtn->setAccessibleName(m_liveTailBtn->toolTip());
    }
    if (m_searchToggleBtn) {
        m_searchToggleBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Buscar"));
        const QStringList searchTooltip = {
            QCoreApplication::translate("InterfaceGrafica", "Barra de Busca"),
            QCoreApplication::translate("InterfaceGrafica", "Alterna a exibição da barra de busca avançada com SIMD/AVX2.")
        };
        m_searchToggleBtn->setToolTip(searchTooltip.join(QLatin1Char('\n')));
        m_searchToggleBtn->setAccessibleName(m_searchToggleBtn->toolTip());
    }
    if (m_timelineToggleBtn) {
        m_timelineToggleBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Timeline"));
        const QStringList timelineTooltip = {
            QCoreApplication::translate("InterfaceGrafica", "Timeline de Distribuição"),
            QCoreApplication::translate("InterfaceGrafica", "Exibe a densidade e distribuição temporal dos registros ao longo do arquivo.")
        };
        m_timelineToggleBtn->setToolTip(timelineTooltip.join(QLatin1Char('\n')));
        m_timelineToggleBtn->setAccessibleName(m_timelineToggleBtn->toolTip());
    }
    if (m_jsonToggleBtn) {
        m_jsonToggleBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Inspetor JSON"));
        const QStringList jsonTooltip = {
            QCoreApplication::translate("InterfaceGrafica", "Inspetor de Log & JSON"),
            QCoreApplication::translate("InterfaceGrafica", "Exibe a estrutura JSON formatada e árvore de propriedades da linha selecionada.")
        };
        m_jsonToggleBtn->setToolTip(jsonTooltip.join(QLatin1Char('\n')));
        m_jsonToggleBtn->setAccessibleName(m_jsonToggleBtn->toolTip());
    }
    if (m_genBtn) {
        m_genBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Gerador"));
        const QStringList genTooltip = {
            QCoreApplication::translate("InterfaceGrafica", "Gerador de Logs"),
            QCoreApplication::translate("InterfaceGrafica", "Gera arquivos sintéticos de alta volumetria para testes de performance.")
        };
        m_genBtn->setToolTip(genTooltip.join(QLatin1Char('\n')));
        m_genBtn->setAccessibleName(m_genBtn->toolTip());
    }
    if (m_clearFilterBtn) {
        const QStringList clearTooltip = {
            QCoreApplication::translate("InterfaceGrafica", "Remover Filtro Ativo"),
            QCoreApplication::translate("InterfaceGrafica", "Restaura a visualização de todas as linhas do log.")
        };
        m_clearFilterBtn->setToolTip(clearTooltip.join(QLatin1Char('\n')));
        m_clearFilterBtn->setAccessibleName(m_clearFilterBtn->toolTip());
    }

    if (m_openedFilePath.isEmpty()) {
        if (m_statusFileLabel) {
            m_statusFileLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Nenhum arquivo aberto"));
        }
        if (m_statusSpeedLabel) {
            m_statusSpeedLabel->setText(QStringLiteral("Status: ") + QCoreApplication::translate("InterfaceGrafica", "Pronto"));
        }
    } else if (m_index && m_statusSpeedLabel) {
        if (m_isIndexing) {
            m_statusSpeedLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Indexando: %1% (%2 MB/s)")
                                            .arg(static_cast<int>(m_index->progress() * 100))
                                            .arg(m_index->speedMBs(), 0, 'f', 0));
        } else {
            m_statusSpeedLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Pronto (%1 MB/s)")
                                            .arg(m_index->speedMBs(), 0, 'f', 0));
        }
    }

    updateStatusSizeLabel();
    updateStatusLinesLabel();

    if (m_statusTailLabel) {
        bool running = m_liveTail && m_liveTail->isRunning();
        m_statusTailLabel->setText(running ? QCoreApplication::translate("InterfaceGrafica", "Tail: Ativo (Live)")
                                           : QCoreApplication::translate("InterfaceGrafica", "Tail: Inativo"));
    }

    // Sync radio checkmarks
    if (m_gerenciadorTraducao && m_actionPtBr && m_actionEnUs) {
        QString curLang = m_gerenciadorTraducao->obter_idioma_atual();
        if (curLang.startsWith(QStringLiteral("en"), Qt::CaseInsensitive)) {
            m_actionEnUs->setChecked(true);
        } else {
            m_actionPtBr->setChecked(true);
        }
    }

    // Forward retranslation to all widgets
    if (m_searchBar) m_searchBar->retranslateUi();
    if (m_timeSliceWidget) m_timeSliceWidget->retranslateUi();
    if (m_jsonInspector) m_jsonInspector->retranslateUi();
    if (m_logModel) m_logModel->retranslate();
}

void MainWindow::onActionOpen() {
    QString path = QFileDialog::getOpenFileName(
        this,
        QCoreApplication::translate("InterfaceGrafica", "Abrir Arquivo de Log"),
        m_openedFilePath.isEmpty() ? QString() : QFileInfo(m_openedFilePath).path(),
        QCoreApplication::translate("InterfaceGrafica", "Arquivos de Log (*.log *.txt *.jsonl *.out *.*);;Todos os Arquivos (*.*)")
    );

    if (!path.isEmpty()) {
        openLogFile(path);
    }
}

void MainWindow::openLogFile(const QString& filePath) {
    if (m_liveTail) m_liveTail->stop();
    if (m_searchEngine) m_searchEngine->cancel();

    m_openedFilePath = filePath;
    std::wstring wpath = filePath.toStdWString();

    // 1. Open Memory-Mapped File (Instant Win32 MapViewOfFile)
    if (!m_mmap->open(wpath)) {
        QMessageBox::critical(this, QCoreApplication::translate("InterfaceGrafica", "Erro ao Abrir Arquivo"),
                              QString::fromStdString(m_mmap->lastError()));
        m_openedFilePath.clear();
        m_isIndexing = false;
        m_currentDisplayedLines = 0;
        updateStatusSizeLabel();
        updateStatusLinesLabel();
        return;
    }

    uint64_t fileSize = m_mmap->size();
    m_isIndexing = true;
    m_currentDisplayedLines = 0;

    m_statusFileLabel->setText(QFileInfo(filePath).fileName() + QStringLiteral(" (") + filePath + QStringLiteral(")"));
    updateStatusSizeLabel();
    updateStatusLinesLabel();

    // 2. Set Engine to Model & LogView immediately
    m_logModel->setEngine(m_mmap, m_index);
    m_logView->setLogModel(m_logModel);
    m_minimap->clearMarkers();
    m_timeSliceWidget->clear();
    m_jsonInspector->clear();

    // 3. Start background chunk indexing
    m_statusSpeedLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Indexando em background..."));

    m_index->startIndexing(
        m_mmap->data(),
        m_mmap->size(),
        [this](uint64_t linesSoFar, float progress) {
            // Callback from worker thread: dispatch to Qt main thread
            QMetaObject::invokeMethod(this, [this, linesSoFar, progress]() {
                m_currentDisplayedLines = linesSoFar;
                m_isIndexing = true;
                updateStatusLinesLabel();
                m_statusSpeedLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Indexando: %1% (%2 MB/s)")
                                                .arg(static_cast<int>(progress * 100))
                                                .arg(m_index->speedMBs(), 0, 'f', 0));
                m_logModel->notifyIndexingFinished(linesSoFar);
            }, Qt::QueuedConnection);
        },
        [this](uint64_t finalTotalLines) {
            QMetaObject::invokeMethod(this, [this, finalTotalLines]() {
                m_currentDisplayedLines = finalTotalLines;
                m_isIndexing = false;
                updateStatusLinesLabel();
                updateStatusSizeLabel();
                m_statusSpeedLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Pronto (%1 MB/s)")
                                                .arg(m_index->speedMBs(), 0, 'f', 0));
                m_logModel->notifyIndexingFinished(finalTotalLines);
                triggerBackgroundAnalysis();
            }, Qt::QueuedConnection);
        }
    );

    // 4. If Live Tail button is checked, start watcher
    if (m_liveTailBtn->isChecked()) {
        m_liveTail->start(wpath, fileSize, [this](uint64_t oldSz, uint64_t newSz) {
            QMetaObject::invokeMethod(this, [this, oldSz, newSz]() {
                onFileGrown(oldSz, newSz);
            }, Qt::QueuedConnection);
        });
        m_statusTailLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Tail: Ativo (Live)"));
    }
}

void MainWindow::triggerBackgroundAnalysis() {
    if (!m_mmap || !m_mmap->isOpen() || !m_index) return;

    // Build timeline distribution in background
    std::jthread([this]() {
        auto res = Engine::TimeSliceScanner::buildDistribution(
            m_mmap->data(),
            m_mmap->size(),
            *m_index,
            70
        );

        QMetaObject::invokeMethod(this, [this, res = std::move(res)]() {
            m_timeSliceWidget->setTimeSliceData(res);
        }, Qt::QueuedConnection);
    }).detach();
}

void MainWindow::onFileGrown(uint64_t oldSize, uint64_t newSize) {
    if (!m_mmap || !m_index) return;

    uint64_t oldLines = m_index->totalLines();

    // 1. Remap memory-mapped file
    m_mmap->remap(newSize);

    // 2. Append newly written byte range to indexer
    m_index->appendRange(m_mmap->data(), oldSize, newSize, [this, oldLines](uint64_t newLines) {
        m_logModel->notifyLinesAppended(oldLines, newLines);

        // Auto-scroll to end if live tail is active
        if (m_liveTailBtn->isChecked()) {
            m_logView->scrollToRow(static_cast<int>(newLines - 1));
        }

        m_currentDisplayedLines = newLines;
        updateStatusSizeLabel();
        updateStatusLinesLabel();
    });
}

void MainWindow::onActionLiveTailToggled(bool active) {
    if (!m_liveTail) return;

    m_liveTailBtn->setText(active ? QCoreApplication::translate("InterfaceGrafica", "Live Tail: ON")
                                  : QCoreApplication::translate("InterfaceGrafica", "Live Tail: OFF"));

    if (m_actionLiveTail && m_actionLiveTail->isChecked() != active) {
        m_actionLiveTail->setChecked(active);
    }

    if (active) {
        if (!m_openedFilePath.isEmpty()) {
            m_liveTail->start(m_openedFilePath.toStdWString(), m_mmap->size(), [this](uint64_t oldSz, uint64_t newSz) {
                QMetaObject::invokeMethod(this, [this, oldSz, newSz]() {
                    onFileGrown(oldSz, newSz);
                }, Qt::QueuedConnection);
            });
            m_statusTailLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Tail: Ativo (Live)"));
            // Jump to end
            if (m_logModel->rowCount() > 0) {
                m_logView->scrollToRow(m_logModel->rowCount() - 1);
            }
        }
    } else {
        m_liveTail->stop();
        m_statusTailLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Tail: Inativo"));
    }
}

void MainWindow::onActionToggleSearch() {
    bool isVis = !m_searchBar->isVisible();
    m_searchBar->setVisible(isVis);
    m_searchToggleBtn->setChecked(isVis);
    if (m_actionToggleSearch) m_actionToggleSearch->setChecked(isVis);
    if (isVis) {
        m_searchBar->setFocus();
    }
}

void MainWindow::onActionToggleTimeline() {
    bool isVis = !m_timeSliceWidget->isVisible();
    m_timeSliceWidget->setVisible(isVis);
    m_timelineToggleBtn->setChecked(isVis);
    if (m_actionToggleTimeline) m_actionToggleTimeline->setChecked(isVis);
}

void MainWindow::onActionToggleJsonInspector() {
    bool isVis = !m_jsonInspector->isVisible();
    m_jsonInspector->setVisible(isVis);
    m_jsonToggleBtn->setChecked(isVis);
    if (m_actionToggleJsonInspector) m_actionToggleJsonInspector->setChecked(isVis);
}

void MainWindow::onActionGenerateLog() {
    GerenciadorDialogos::instance().exibir_gerador(nullptr);
    if (auto* gen = GerenciadorDialogos::instance().obter_gerador_dialogo()) {
        connect(gen, &LogGeneratorDialog::logGenerated, this, &MainWindow::openLogFile, Qt::UniqueConnection);
    }
}

void MainWindow::onSearchTriggered(const Engine::SearchOptions& options) {
    if (!m_mmap || !m_mmap->isOpen() || !m_index) return;

    m_searchEngine->cancel();

    m_searchEngine->startSearch(
        m_mmap->data(),
        m_mmap->size(),
        *m_index,
        options,
        [this](const std::vector<Engine::SearchMatch>& /*matches*/, float progress) {
            QMetaObject::invokeMethod(this, [this, progress]() {
                m_searchBar->setProgress(progress);
            }, Qt::QueuedConnection);
        },
        [this](const std::vector<Engine::SearchMatch>& allMatches, double elapsed) {
            QMetaObject::invokeMethod(this, [this, allMatches, elapsed]() {
                onSearchFinished(allMatches, elapsed);
            }, Qt::QueuedConnection);
        }
    );
}

void MainWindow::onSearchFinished(const std::vector<Engine::SearchMatch>& matches, double elapsed) {
    m_activeMatches = matches;
    m_currentMatchIndex = matches.empty() ? -1 : 0;

    m_searchBar->setMatchResults(m_currentMatchIndex, static_cast<int>(matches.size()), elapsed);

    // Update minimap with match markers
    std::vector<uint64_t> matchLines;
    matchLines.reserve(matches.size());
    for (const auto& m : matches) {
        matchLines.push_back(m.lineIndex);
    }
    m_minimap->setSearchMarkers(matchLines);

    if (m_filterModeActive && !matches.empty()) {
        m_logModel->setFilteredLines(std::move(matchLines), QStringLiteral("Busca"));
    }

    if (!matches.empty()) {
        m_logView->scrollToRow(static_cast<int>(matches[0].lineIndex));
    }
}

void MainWindow::onClearFilter() {
    m_logView->logDelegate()->clearSearchHighlight();
    m_logView->viewport()->update();
    m_logModel->clearFilter();
    m_minimap->clearMarkers();
    m_activeMatches.clear();
    m_currentMatchIndex = -1;
}

void MainWindow::onTimeSliceClicked(uint64_t lineIndex) {
    m_logView->scrollToRow(static_cast<int>(lineIndex));
}

void MainWindow::onLineSelected(uint64_t realLine, const QString& fullLine) {
    if (!m_mmap || !m_index) return;
    auto span = m_index->getLineSpan(m_mmap->data(), m_mmap->size(), realLine);
    m_jsonInspector->inspectLine(realLine, span.offset, fullLine);
}

void MainWindow::onSeekRequested(int row) {
    m_logView->scrollToRow(row);
}

void MainWindow::updateSystemStats() {
    m_statusRamLabel->setText(QStringLiteral("RAM: %1").arg(Win32DwmHelper::getProcessMemoryUsageString()));
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    const auto urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString localPath = urls.first().toLocalFile();
        if (!localPath.isEmpty()) {
            openLogFile(localPath);
        }
    }
}

} // namespace Munin::UI
