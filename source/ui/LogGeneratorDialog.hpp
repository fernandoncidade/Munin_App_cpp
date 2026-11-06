#pragma once

#include <QDialog>
#include <atomic>
#include <thread>

class QComboBox;
class QProgressBar;
class QLabel;
class QPushButton;
class QLineEdit;

namespace Munin::UI {

class LogGeneratorDialog : public QDialog {
    Q_OBJECT

public:
    explicit LogGeneratorDialog(QWidget* parent = nullptr);
    ~LogGeneratorDialog() override;

    [[nodiscard]] QString generatedFilePath() const { return m_generatedPath; }

    void retranslateUi();
    void applyTheme(bool isDark);

protected:
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;

signals:
    void logGenerated(const QString& filePath);

private slots:
    void onGenerateClicked();
    void onCancelClicked();

private:
    void generateWorker(std::wstring filePath, uint64_t targetBytes);

    QLabel* m_infoLabel{nullptr};
    QLabel* m_sizeLabel{nullptr};
    QLabel* m_pathLabel{nullptr};
    QPushButton* m_browseBtn{nullptr};
    QComboBox* m_sizeCombo{nullptr};
    QLineEdit* m_pathEdit{nullptr};
    QProgressBar* m_progressBar{nullptr};
    QLabel* m_statusLabel{nullptr};
    QPushButton* m_generateBtn{nullptr};
    QPushButton* m_cancelBtn{nullptr};

    std::atomic<bool> m_cancelRequested{false};
    std::atomic<uint64_t> m_bytesWritten{0};
    std::jthread m_workerThread;
    QString m_generatedPath;
};

} // namespace Munin::UI
