#pragma once

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QVector>

class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QWidget;

namespace mocks::translations {

struct TranslationTarget final {
    QString id;
    QString display_name;
    QStringList file_patterns;
    QString translations_relative_path;
};

class TranslationCompileDialog final : public QDialog {
public:
    explicit TranslationCompileDialog(QWidget *parent = nullptr,
                                      const QString &initial_target_id = QString());

private:
    struct CompileSummary {
        int discovered_files = 0;
        int succeeded_files = 0;
        int failed_files = 0;

        [[nodiscard]] bool has_failures() const { return failed_files > 0; }
    };

    void setup_ui();
    void populate_targets(const QString &initial_target_id);
    void run_compilation();
    void append_log(const QString &message);
    QString locate_project_root() const;
    QString locate_lrelease_executable() const;
    QString current_translations_directory() const;
    QString effective_source_directory() const;
    QString effective_output_directory() const;
    TranslationTarget current_target() const;
    void refresh_context_labels();
    void sync_path_fields_from_target();
    void browse_source_directory();
    void browse_output_directory();
    void reset_source_directory();
    void reset_output_directory();
    CompileSummary compile_current_target(const QString &source_directory,
                                          const QString &output_directory);

    QLabel *status_label_ = nullptr;
    QLabel *translations_dir_label_ = nullptr;
    QLabel *lrelease_label_ = nullptr;
    QComboBox *target_combo_ = nullptr;
    QLineEdit *source_dir_edit_ = nullptr;
    QLineEdit *output_dir_edit_ = nullptr;
    QPlainTextEdit *log_output_ = nullptr;
    QPushButton *compile_button_ = nullptr;

    QVector<TranslationTarget> targets_;
    QString project_root_;
    QString lrelease_executable_;
    bool source_dir_custom_ = false;
    bool output_dir_custom_ = false;
};

}  // namespace mocks::translations
