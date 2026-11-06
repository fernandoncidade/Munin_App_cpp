#include "LogGeneratorDialog.hpp"
#include "ButtonUtils.hpp"
#include "GerenciadorDialogos.hpp"
#include "ThemeManager.hpp"
#include "Win32DwmHelper.hpp"
#include "../utils/ApplicationPathUtils.hpp"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <QAbstractItemView>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QShowEvent>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

namespace Munin::UI {

LogGeneratorDialog::LogGeneratorDialog(QWidget* /*parent*/)
    : QDialog(nullptr) {
    setWindowIcon(get_app_icon());
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                   Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    setModal(false);
    setAttribute(Qt::WA_DeleteOnClose);
    resize(480, 240);

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    m_infoLabel = new QLabel(this);
    m_infoLabel->setWordWrap(true);
    QPalette infoPal = m_infoLabel->palette();
    infoPal.setColor(QPalette::WindowText, palette().color(QPalette::PlaceholderText));
    m_infoLabel->setPalette(infoPal);
    layout->addWidget(m_infoLabel);

    auto* formLayout = new QVBoxLayout();

    // Size combo
    auto* sizeLayout = new QHBoxLayout();
    m_sizeLabel = new QLabel(this);
    sizeLayout->addWidget(m_sizeLabel);
    m_sizeCombo = new QComboBox(this);
    m_sizeCombo->addItem(QString(), QVariant(static_cast<qulonglong>(100ULL * 1024ULL * 1024ULL)));
    m_sizeCombo->addItem(QString(), QVariant(static_cast<qulonglong>(500ULL * 1024ULL * 1024ULL)));
    m_sizeCombo->addItem(QString(), QVariant(static_cast<qulonglong>(1ULL * 1024ULL * 1024ULL * 1024ULL)));
    m_sizeCombo->addItem(QString(), QVariant(static_cast<qulonglong>(5ULL * 1024ULL * 1024ULL * 1024ULL)));
    m_sizeCombo->addItem(QString(), QVariant(static_cast<qulonglong>(10ULL * 1024ULL * 1024ULL * 1024ULL)));
    sizeLayout->addWidget(m_sizeCombo, 1);
    formLayout->addLayout(sizeLayout);

    // Path edit
    auto* pathLayout = new QHBoxLayout();
    m_pathLabel = new QLabel(this);
    pathLayout->addWidget(m_pathLabel);
    QString defaultPath = QDir::temp().filePath(QStringLiteral("munin_benchmark.log"));
    m_pathEdit = new QLineEdit(defaultPath, this);
    pathLayout->addWidget(m_pathEdit, 1);

    m_browseBtn = new QPushButton(this);
    configure_license_action_button(m_browseBtn, 90);
    m_browseBtn->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    connect(m_browseBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getSaveFileName(
            this,
            QCoreApplication::translate("InterfaceGrafica", "Salvar Arquivo de Log de Teste"),
            m_pathEdit->text(),
            QCoreApplication::translate("InterfaceGrafica", "Arquivos de Log (*.log);;Todos os Arquivos (*.*)")
        );
        if (!path.isEmpty()) {
            m_pathEdit->setText(path);
        }
    });
    pathLayout->addWidget(m_browseBtn);
    formLayout->addLayout(pathLayout);

    layout->addLayout(formLayout);

    // Progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    layout->addWidget(m_progressBar);

    m_statusLabel = new QLabel(this);
    layout->addWidget(m_statusLabel);

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_cancelBtn = new QPushButton(this);
    configure_license_action_button(m_cancelBtn, 90);
    m_cancelBtn->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(m_cancelBtn, &QPushButton::clicked, this, &LogGeneratorDialog::onCancelClicked);
    btnLayout->addWidget(m_cancelBtn);

    m_generateBtn = new QPushButton(this);
    m_generateBtn->setObjectName(QStringLiteral("generateBtn"));
    configure_license_action_button(m_generateBtn, 100);
    m_generateBtn->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    connect(m_generateBtn, &QPushButton::clicked, this, &LogGeneratorDialog::onGenerateClicked);
    btnLayout->addWidget(m_generateBtn);

    layout->addLayout(btnLayout);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &LogGeneratorDialog::applyTheme);
    applyTheme(ThemeManager::instance().isDark());

    retranslateUi();
}

void LogGeneratorDialog::retranslateUi() {
    const QString idioma = GerenciadorDialogos::instance().obter_idioma_atual();
    const bool isPt = !idioma.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);
    setWindowTitle(isPt ? QStringLiteral("Gerador de Logs Massivos (Benchmark)")
                        : QStringLiteral("Massive Log Generator (Benchmark)"));
    if (m_infoLabel) {
        m_infoLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Gere arquivos de log sintéticos realistas com timestamps ISO-8601, JSON lines, IDs de thread e múltiplos níveis para testar a abertura instantânea e a paginação Win32."));
    }
    if (m_sizeLabel) {
        m_sizeLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Tamanho do Arquivo:"));
    }
    if (m_sizeCombo) {
        m_sizeCombo->setItemText(0, QCoreApplication::translate("InterfaceGrafica", "100 MB (~800.000 linhas)"));
        m_sizeCombo->setItemText(1, QCoreApplication::translate("InterfaceGrafica", "500 MB (~4.000.000 linhas)"));
        m_sizeCombo->setItemText(2, QCoreApplication::translate("InterfaceGrafica", "1 GB (~8.000.000 linhas)"));
        m_sizeCombo->setItemText(3, QCoreApplication::translate("InterfaceGrafica", "5 GB (~40.000.000 linhas)"));
        m_sizeCombo->setItemText(4, QCoreApplication::translate("InterfaceGrafica", "10 GB (~80.000.000 linhas)"));
    }
    if (m_pathLabel) {
        m_pathLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Caminho de Destino:"));
    }
    if (m_browseBtn) {
        m_browseBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Procurar..."));
    }
    if (m_statusLabel && m_progressBar && m_progressBar->value() == 0) {
        m_statusLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Pronto para gerar."));
    }
    if (m_cancelBtn) {
        m_cancelBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Cancelar"));
    }
    if (m_generateBtn) {
        m_generateBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Gerar Log"));
    }
}

void LogGeneratorDialog::applyTheme(bool isDark) {
    const QPalette pal = isDark ? ThemeManager::instance().createDarkPalette()
                                : ThemeManager::instance().createLightPalette();
    setPalette(pal);

    if (m_infoLabel) {
        m_infoLabel->setPalette(pal);
    }
    if (m_sizeCombo) {
        m_sizeCombo->setPalette(pal);
        if (m_sizeCombo->view()) {
            m_sizeCombo->view()->setPalette(pal);
        }
    }
    if (m_pathEdit) {
        m_pathEdit->setPalette(pal);
    }
    if (m_sizeLabel) {
        m_sizeLabel->setPalette(pal);
    }
    if (m_pathLabel) {
        m_pathLabel->setPalette(pal);
    }
    if (m_statusLabel) {
        m_statusLabel->setPalette(pal);
    }
    if (m_browseBtn) {
        m_browseBtn->setPalette(pal);
    }
    if (m_progressBar) {
        m_progressBar->setPalette(pal);
    }
    if (m_cancelBtn) {
        m_cancelBtn->setPalette(pal);
    }
    if (m_generateBtn) {
        m_generateBtn->setPalette(pal);
    }

    Win32DwmHelper::applyTitleBarTheme(this, isDark);
    update();
}

void LogGeneratorDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    Win32DwmHelper::applyTitleBarTheme(this, ThemeManager::instance().isDark());
}

void LogGeneratorDialog::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QDialog::changeEvent(event);
}

LogGeneratorDialog::~LogGeneratorDialog() {
    m_cancelRequested.store(true);
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void LogGeneratorDialog::onCancelClicked() {
    m_cancelRequested.store(true);
    reject();
}

void LogGeneratorDialog::onGenerateClicked() {
    uint64_t targetBytes = m_sizeCombo->currentData().toULongLong();
    QString filePath = m_pathEdit->text().trimmed();

    if (filePath.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Aviso"), QStringLiteral("Por favor, informe um caminho de destino válido."));
        return;
    }

    m_generateBtn->setEnabled(false);
    m_sizeCombo->setEnabled(false);
    m_pathEdit->setEnabled(false);
    m_cancelRequested.store(false);
    m_bytesWritten.store(0);
    m_generatedPath = filePath;

    m_statusLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Gerando arquivo de log em thread dedicada..."));

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, timer, targetBytes]() {
        uint64_t written = m_bytesWritten.load();
        int prog = static_cast<int>((static_cast<double>(written) / static_cast<double>(targetBytes)) * 100.0);
        m_progressBar->setValue(std::min(100, prog));

        double writtenMB = static_cast<double>(written) / (1024.0 * 1024.0);
        double totalMB = static_cast<double>(targetBytes) / (1024.0 * 1024.0);
        m_statusLabel->setText(QStringLiteral("Escritos: %1 MB / %2 MB (%3%)")
                                   .arg(writtenMB, 0, 'f', 1)
                                   .arg(totalMB, 0, 'f', 1)
                                   .arg(prog));

        if (written >= targetBytes || m_cancelRequested.load()) {
            timer->stop();
            timer->deleteLater();
            m_generateBtn->setEnabled(true);
            m_sizeCombo->setEnabled(true);
            m_pathEdit->setEnabled(true);

            if (!m_cancelRequested.load()) {
                m_statusLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Geração concluída com sucesso!"));
                emit logGenerated(m_generatedPath);
                accept();
            }
        }
    });
    timer->start(100);

    m_workerThread = std::jthread([this, path = filePath.toStdWString(), targetBytes]() {
        generateWorker(path, targetBytes);
    });
}

void LogGeneratorDialog::generateWorker(std::wstring filePath, uint64_t targetBytes) {
    FILE* fp = nullptr;
    _wfopen_s(&fp, filePath.c_str(), L"wb");
    if (!fp) return;

    // Set large 2MB buffer for ultra-fast disk writes
    setvbuf(fp, nullptr, _IOFBF, 2 * 1024 * 1024);

    const char* levels[] = {"INFO", "DEBUG", "WARN", "ERROR", "TRACE", "INFO", "INFO", "WARN"};
    const char* endpoints[] = {"/api/v1/checkout", "/api/v2/orders", "/auth/login", "/metrics", "/health", "/payment/gateway"};

    uint64_t written = 0;
    uint64_t lineCount = 0;
    char buffer[512];

    while (written < targetBytes && !m_cancelRequested.load(std::memory_order_relaxed)) {
        int lvlIdx = static_cast<int>(lineCount % 8);
        int epIdx = static_cast<int>(lineCount % 6);
        int threadId = static_cast<int>(lineCount % 16) + 1;
        int userId = static_cast<int>(lineCount % 9999) + 1000;
        int latency = static_cast<int>((lineCount * 7) % 350) + 5;

        // Realistic ISO-8601 timestamps ticking forward
        int hour = static_cast<int>((lineCount / 3600) % 24);
        int min = static_cast<int>((lineCount / 60) % 60);
        int sec = static_cast<int>(lineCount % 60);

        int len = snprintf(
            buffer, sizeof(buffer),
            "2026-10-13T%02d:%02d:%02d.451Z [thread-%02d] 192.168.1.%d [%s] %s - {\"user_id\":%d,\"latency_ms\":%d,\"status\":%d}\n",
            hour, min, sec,
            threadId,
            (threadId * 13) % 250 + 1,
            levels[lvlIdx],
            endpoints[epIdx],
            userId,
            latency,
            (lvlIdx == 3 ? 500 : (lvlIdx == 2 ? 404 : 200))
        );

        if (len > 0) {
            size_t w = fwrite(buffer, 1, static_cast<size_t>(len), fp);
            written += w;
            ++lineCount;

            if ((lineCount % 5000) == 0) {
                m_bytesWritten.store(written, std::memory_order_relaxed);
            }
        }
    }

    m_bytesWritten.store(written, std::memory_order_relaxed);
    fclose(fp);
}

} // namespace Munin::UI
