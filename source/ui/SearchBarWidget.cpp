#include "SearchBarWidget.hpp"
#include "ButtonUtils.hpp"
#include "ThemeManager.hpp"

#include <QCoreApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>

namespace Munin::UI {

SearchBarWidget::SearchBarWidget(QWidget* parent)
    : QWidget(parent) {
    setObjectName(QStringLiteral("searchBarRoot"));

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 6, 8, 6);
    mainLayout->setSpacing(6);

    auto* searchIcon = new QLabel(this);
    searchIcon->setPixmap(style()->standardIcon(QStyle::SP_FileDialogContentsView).pixmap(16, 16));
    mainLayout->addWidget(searchIcon);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setPlaceholderText(QCoreApplication::translate("InterfaceGrafica", "Buscar no log... (Pressione Enter para busca JIT multi-core)"));
    apply_input_bg(m_searchEdit, palette().color(QPalette::Base));
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &SearchBarWidget::onSearchSubmitted);
    mainLayout->addWidget(m_searchEdit, 1);

    m_regexBtn = new QPushButton(QStringLiteral(".*"), this);
    m_regexBtn->setCheckable(true);
    configure_license_action_button(m_regexBtn, 32);
    connect(m_regexBtn, &QPushButton::toggled, this, &SearchBarWidget::onOptionChanged);
    mainLayout->addWidget(m_regexBtn);

    m_caseBtn = new QPushButton(QStringLiteral("Aa"), this);
    m_caseBtn->setCheckable(true);
    configure_license_action_button(m_caseBtn, 32);
    connect(m_caseBtn, &QPushButton::toggled, this, &SearchBarWidget::onOptionChanged);
    mainLayout->addWidget(m_caseBtn);

    m_invertBtn = new QPushButton(QStringLiteral("!"), this);
    m_invertBtn->setCheckable(true);
    configure_license_action_button(m_invertBtn, 28);
    connect(m_invertBtn, &QPushButton::toggled, this, &SearchBarWidget::onOptionChanged);
    mainLayout->addWidget(m_invertBtn);

    m_filterBtn = new QPushButton(QCoreApplication::translate("InterfaceGrafica", "Filtrar Linhas"), this);
    m_filterBtn->setCheckable(true);
    configure_license_action_button(m_filterBtn, 100);
    connect(m_filterBtn, &QPushButton::toggled, this, &SearchBarWidget::filterModeToggled);
    mainLayout->addWidget(m_filterBtn);

    m_prevBtn = new QToolButton(this);
    configure_tool_button(m_prevBtn, Qt::UpArrow, QCoreApplication::translate("InterfaceGrafica", "Ocorrência Anterior (Shift+Enter)"));
    connect(m_prevBtn, &QToolButton::clicked, this, &SearchBarWidget::prevMatchTriggered);
    mainLayout->addWidget(m_prevBtn);

    m_nextBtn = new QToolButton(this);
    configure_tool_button(m_nextBtn, Qt::DownArrow, QCoreApplication::translate("InterfaceGrafica", "Próxima Ocorrência (Enter)"));
    connect(m_nextBtn, &QToolButton::clicked, this, &SearchBarWidget::nextMatchTriggered);
    mainLayout->addWidget(m_nextBtn);

    m_statusLabel = new QLabel(QCoreApplication::translate("InterfaceGrafica", "Pronto"), this);
    mainLayout->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setFixedWidth(80);
    m_progressBar->setFixedHeight(12);
    m_progressBar->setTextVisible(false);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);

    m_closeBtn = new QPushButton(this);
    m_closeBtn->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    configure_license_action_button(m_closeBtn, 28);
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
        m_searchEdit->clear();
        m_filterBtn->setChecked(false);
        m_statusLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Pronto"));
        emit clearTriggered();
    });
    mainLayout->addWidget(m_closeBtn);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &SearchBarWidget::applyTheme);
    applyTheme(ThemeManager::instance().isDark());
    retranslateUi();
}

void SearchBarWidget::applyTheme(bool isDark) {
    const QPalette pal = isDark ? ThemeManager::instance().createDarkPalette()
                                : ThemeManager::instance().createLightPalette();
    setPalette(pal);
    if (m_searchEdit) {
        m_searchEdit->setPalette(pal);
    }
    update();
}

void SearchBarWidget::retranslateUi() {
    m_searchEdit->setPlaceholderText(QCoreApplication::translate("InterfaceGrafica", "Buscar no log... (Pressione Enter para busca JIT multi-core)"));
    m_regexBtn->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Usar Expressão Regular (Regex JIT)"));
    m_regexBtn->setAccessibleName(m_regexBtn->toolTip());
    m_caseBtn->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Diferenciar Maiúsculas / Minúsculas (Case Sensitive)"));
    m_caseBtn->setAccessibleName(m_caseBtn->toolTip());
    m_invertBtn->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Inverter correspondência (Não contém)"));
    m_invertBtn->setAccessibleName(m_invertBtn->toolTip());
    m_filterBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Filtrar Linhas"));
    m_filterBtn->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Modo Filtro: exibir apenas linhas que correspondem à busca"));
    m_filterBtn->setAccessibleName(m_filterBtn->toolTip());
    m_prevBtn->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Ocorrência Anterior (Shift+Enter)"));
    m_prevBtn->setAccessibleName(m_prevBtn->toolTip());
    m_nextBtn->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Próxima Ocorrência (Enter)"));
    m_nextBtn->setAccessibleName(m_nextBtn->toolTip());
    m_closeBtn->setToolTip(QCoreApplication::translate("InterfaceGrafica", "Limpar e Fechar Busca (Esc)"));
    m_closeBtn->setAccessibleName(m_closeBtn->toolTip());
    if (m_statusLabel->text().isEmpty() || m_statusLabel->text() == QStringLiteral("Pronto") || m_statusLabel->text() == QStringLiteral("Ready")) {
        m_statusLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Pronto"));
    }
}

void SearchBarWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void SearchBarWidget::setQuery(const QString& query) {
    m_searchEdit->setText(query);
    onSearchSubmitted();
}

Engine::SearchOptions SearchBarWidget::collectOptions() const {
    Engine::SearchOptions opts;
    opts.query = m_searchEdit->text().toStdString();
    opts.isRegex = m_regexBtn->isChecked();
    opts.caseSensitive = m_caseBtn->isChecked();
    opts.invertMatch = m_invertBtn->isChecked();
    return opts;
}

void SearchBarWidget::onSearchSubmitted() {
    QString q = m_searchEdit->text();
    if (q.isEmpty()) {
        m_statusLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Busca vazia."));
        emit clearTriggered();
        return;
    }
    emit searchTriggered(collectOptions());
}

void SearchBarWidget::onOptionChanged() {
    if (!m_searchEdit->text().isEmpty()) {
        onSearchSubmitted();
    }
}

void SearchBarWidget::setMatchResults(int currentIndex, int totalMatches, double elapsedSeconds) {
    if (totalMatches <= 0) {
        m_statusLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Nenhuma ocorrência encontrada."));
    } else {
        QString text = QStringLiteral("%1 / %2 (%3s)")
            .arg(currentIndex + 1)
            .arg(totalMatches)
            .arg(QString::number(elapsedSeconds, 'f', 2));
        m_statusLabel->setText(text);
    }
}

void SearchBarWidget::setSearching(bool searching) {
    m_progressBar->setVisible(searching);
    if (searching) {
        m_statusLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Buscando..."));
        m_progressBar->setValue(0);
    }
}

void SearchBarWidget::setProgress(float progress) {
    m_progressBar->setValue(static_cast<int>(progress * 100.0f));
}

} // namespace Munin::UI
