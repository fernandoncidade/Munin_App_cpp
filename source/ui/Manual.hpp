#pragma once

#include <QDialog>
#include <QList>
#include <QString>
#include <QTextCursor>
#include <QVector>

class QEvent;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QShowEvent;
class QSplitter;
class QTextBrowser;
class QToolButton;

namespace Munin::UI {

struct ManualSection {
    QString id;
    QString title;
    QString htmlContent;
};

/**
 * @brief Caixa de diálogo "Manual de Instruções" implementada com o tema nativo e padrão do Qt6,
 * fornecendo documentação operacional completa para a interface gráfica e o utilitário de linha de comando (CLI).
 */
class ManualDialog : public QDialog {
    Q_OBJECT

public:
    explicit ManualDialog(QWidget* parent = nullptr, const QString& idioma = QStringLiteral("pt_BR"));
    ~ManualDialog() override = default;

    void atualizar_traducao(const QString& idioma = QString());
    void applyTheme(bool isDark);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    void buildSections(const QString& idioma);
    void renderDocument();
    void updateSearchTexts();
    void rebuildSearch(const QString& text);
    void navigateSearch(int step);
    void updateSearchButtons();
    void updateSearchHighlights();

    QLabel* search_label_ = nullptr;
    QLineEdit* search_edit_ = nullptr;
    QToolButton* search_previous_button_ = nullptr;
    QToolButton* search_next_button_ = nullptr;

    QSplitter* splitter_ = nullptr;
    QListWidget* toc_list_ = nullptr;
    QTextBrowser* browser_ = nullptr;
    QPushButton* btn_ok_ = nullptr;

    QString current_lang_{QStringLiteral("pt_BR")};
    QVector<ManualSection> sections_;

    QList<QTextCursor> search_matches_;
    int search_current_index_ = -1;
};

void exibir_manual(QWidget* parent, const QString& idioma = QStringLiteral("pt_BR"));

} // namespace Munin::UI
