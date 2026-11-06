#pragma once

#include <QDialog>
#include <QHash>
#include <QPalette>
#include <QPointer>
#include <QString>
#include <QVector>

class QLabel;
class QLineEdit;
class QPushButton;
class QResizeEvent;
class QShowEvent;
class QTabWidget;
class QTextBrowser;
class QToolButton;
class QUrl;

namespace Munin::UI {

struct DocumentoSobre {
    QString titulo;
    QString texto;
};

/**
 * @brief Caixa de diálogo "Sobre" construída estritamente com os métodos,
 * estilo, design e arquitetura nativa do Qt6 do projeto Linceu_Lighthouse_cpp,
 * com suporte completo a tradução dinâmica em tempo de execução.
 */
class SobreDialog : public QDialog {
    Q_OBJECT

public:
    explicit SobreDialog(
        QWidget* parent,
        const QString& titulo,
        const QString& texto_fixo,
        const QString& texto_history,
        const QString& detalhes,
        const QString& licencas,
        const QString& sites_licencas,
        const QString& show_history_text = QString(),
        const QString& hide_history_text = QString(),
        const QString& show_details_text = QString(),
        const QString& hide_details_text = QString(),
        const QString& show_licenses_text = QString(),
        const QString& hide_licenses_text = QString(),
        const QString& ok_text = QStringLiteral("OK"),
        const QString& site_oficial_text = QString(),
        const QString& avisos = QString(),
        const QString& show_notices_text = QString(),
        const QString& hide_notices_text = QString(),
        const QString& privacy_policy = QString(),
        const QString& show_privacy_policy_text = QString(),
        const QString& hide_privacy_policy_text = QString(),
        const QString& info_not_available_text = QStringLiteral("Information not available"),
        const QString& release_notes = QString(),
        const QString& show_release_notes_text = QString(),
        const QString& hide_release_notes_text = QString(),
        const QString& clc = QString(),
        const QString& copyright = QString());

    ~SobreDialog() override = default;

    void retranslateUi(const QString& novo_idioma = QString());

public slots:
    void on_idioma_alterado(const QString& novo_idioma = QString());
    void applyTheme(bool isDark);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void configure_browser(QTextBrowser* browser);
    void apply_text_or_placeholder(QTextBrowser* browser,
                                   const QString& content,
                                   const QString& info_not_available_text,
                                   const QPalette& pal = QPalette());
    void register_license_document(const QString& doc_id,
                                   const QString& displayed_path,
                                   const QString& license_title,
                                   const QString& content);
    void handle_anchor_click(const QUrl& url);
    void open_license_document(const QString& doc_id);
    void reload_browser_contents(const QPalette& pal = QPalette());
    int calculate_fixed_label_height(int available_width) const;
    void refresh_fixed_label_geometry();
    void update_tab_labels(int current_index);
    void update_search_texts();
    void apply_about_search();
    void navigate_about_search(int step);
    void update_about_search_buttons();
    void reset_about_search_position();

private:
    QLabel* fixed_label_ = nullptr;
    QLabel* search_label_ = nullptr;
    QLineEdit* search_edit_ = nullptr;
    QToolButton* search_previous_button_ = nullptr;
    QToolButton* search_next_button_ = nullptr;
    QTabWidget* tabs_ = nullptr;
    QPushButton* ok_button_ = nullptr;
    QTextBrowser* about_browser_ = nullptr;
    QTextBrowser* history_browser_ = nullptr;
    QTextBrowser* eula_browser_ = nullptr;
    QTextBrowser* clc_browser_ = nullptr;
    QTextBrowser* copyright_browser_ = nullptr;
    QTextBrowser* licencas_browser_ = nullptr;
    QTextBrowser* avisos_browser_ = nullptr;
    QTextBrowser* privacidade_browser_ = nullptr;
    QTextBrowser* release_notes_browser_ = nullptr;
    QTextBrowser* detalhes_browser_ = nullptr;
    QString current_lang_{QStringLiteral("pt_BR")};
    QString info_not_available_text_;
    QString sites_licencas_;
    QString site_oficial_text_;
    QHash<QString, QString> displayed_path_to_doc_id_;
    QHash<QString, QString> license_document_titles_;
    QHash<QString, QString> license_document_contents_;
    QVector<QPair<QString, QString>> tab_labels_;
    QTextBrowser* search_current_browser_ = nullptr;
    int search_current_start_ = -1;
};

void exibir_sobre(QWidget* parent = nullptr, const QString& idioma = QStringLiteral("pt_BR"));

} // namespace Munin::UI
