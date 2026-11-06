#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace mocks::translations {

class TranslationCompilerTerminal final {
public:
    int run(const QStringList &arguments);

private:
    struct TranslationTarget {
        QString id;
        QString display_name;
        QStringList file_patterns;
        QString translations_relative_path;
    };

    struct CompileSummary {
        int discovered_files = 0;
        int succeeded_files = 0;
        int failed_files = 0;
    };

    struct RunRequest {
        QString requested_target_id;
        bool compile_all = true;
    };

    RunRequest parse_arguments(const QStringList &arguments) const;
    QVector<TranslationTarget> available_targets() const;
    QVector<TranslationTarget> requested_targets(const RunRequest &request) const;
    QString locate_project_root() const;
    QString locate_lrelease_executable() const;
    QString translations_directory(const QString &project_root, const TranslationTarget &target) const;
    CompileSummary compile_translation_files(const TranslationTarget &target,
                                             const QString &translations_dir,
                                             const QString &lrelease_executable) const;
    void write_line(const QString &message) const;
    void write_error(const QString &message) const;
};

}  // namespace mocks::translations
