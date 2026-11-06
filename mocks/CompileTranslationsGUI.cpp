#include "CompileTranslationsGUI.hpp"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QScopeGuard>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QStringList>
#include <QTextCursor>
#include <QVBoxLayout>

namespace {

QString normalize_target_id(QString value)
{
    value = value.trimmed().toLower();
    value.remove(QLatin1Char('_'));
    value.remove(QLatin1Char('-'));
    value.remove(QLatin1Char(' '));

    if (value.contains(QStringLiteral("munin")) || value == QStringLiteral("host") || value == QStringLiteral("app")) {
        return QStringLiteral("munin");
    }
    if (value == QStringLiteral("todos") || value == QStringLiteral("all")) {
        return QStringLiteral("all");
    }

    return value;
}

QString argument_target_id(const QStringList &arguments)
{
    for (int i = 1; i < arguments.size(); ++i) {
        const QString argument = arguments.at(i);
        if (argument == QStringLiteral("--all") || argument == QStringLiteral("-a")) {
            return QStringLiteral("all");
        }
        if ((argument == QStringLiteral("--module") || argument == QStringLiteral("-m") ||
             argument == QStringLiteral("--target")) &&
            i + 1 < arguments.size()) {
            return normalize_target_id(arguments.at(i + 1));
        }
        if (argument.startsWith(QStringLiteral("--module="))) {
            return normalize_target_id(argument.mid(QStringLiteral("--module=").size()));
        }
        if (argument.startsWith(QStringLiteral("--target="))) {
            return normalize_target_id(argument.mid(QStringLiteral("--target=").size()));
        }
    }

    return {};
}

QVector<mocks::translations::TranslationTarget> available_targets()
{
    return {
        {QStringLiteral("all"),
         QStringLiteral("Todos os Módulos (source/language/translations)"),
         {QStringLiteral("munin_*.ts")},
         QStringLiteral("source/language/translations")},
        {QStringLiteral("munin"),
         QStringLiteral("Munin (Principal)"),
         {QStringLiteral("munin_*.ts")},
         QStringLiteral("source/language/translations")},
    };
}

QString find_project_root_from(const QString &start_path)
{
    QFileInfo info(start_path);
    QDir dir(info.isDir() ? info.absoluteFilePath() : info.absolutePath());

    while (true) {
        const bool has_project_files =
            dir.exists(QStringLiteral("CMakeLists.txt")) && dir.exists(QStringLiteral(".vscode/tasks.json"));
        const bool has_translations = dir.exists(QStringLiteral("source/language/translations"));

        if (has_project_files && has_translations) {
            return dir.absolutePath();
        }

        if (!dir.cdUp()) {
            break;
        }
    }

    return {};
}

QString trim_process_output(const QString &text)
{
    const QString simplified = text.trimmed();
    return simplified.isEmpty() ? QStringLiteral("(sem saida)") : simplified;
}

QString clean_path_from_ui(const QString &path)
{
    const QString normalized = QDir::fromNativeSeparators(path.trimmed());
    return normalized.isEmpty() ? QString() : QDir::cleanPath(normalized);
}

QString display_path_or_status(const QString &path)
{
    return path.trimmed().isEmpty() ? QStringLiteral("Nao localizado") : QDir::toNativeSeparators(path);
}

QString mock_windows_icon_name()
{
#if defined(MOCKS_WINDOWS_ICON_NAME)
    return QStringLiteral(MOCKS_WINDOWS_ICON_NAME);
#else
    return QStringLiteral("munin.ico");
#endif
}

QString find_mock_icon_path(const QString &project_root, const QString &preferred_name = QString())
{
#if defined(Q_OS_WIN)
    const QString primary_icon_name = preferred_name.trimmed().isEmpty() ? mock_windows_icon_name() : preferred_name.trimmed();
    const QStringList candidate_names = {
        primary_icon_name,
        QStringLiteral("munin.ico"),
        QStringLiteral("munin.png"),
        QStringLiteral("munin_300-300.png"),
        QStringLiteral("munin_150-150.png")
    };
    const QDir app_dir(QCoreApplication::applicationDirPath());
    const QDir current_dir(QDir::currentPath());
    const QDir source_dir(QFileInfo(QString::fromLocal8Bit(__FILE__)).absolutePath());

    for (const QString &name : candidate_names) {
        if (name.isEmpty()) {
            continue;
        }
        QStringList paths = {
            app_dir.filePath(QStringLiteral("icons/%1").arg(name)),
            app_dir.filePath(name),
            current_dir.filePath(QStringLiteral("icons/%1").arg(name)),
            current_dir.filePath(QStringLiteral("mocks/icons/%1").arg(name)),
            source_dir.filePath(QStringLiteral("icons/%1").arg(name)),
            source_dir.filePath(QStringLiteral("../source/assets/icons/%1").arg(name)),
        };
        if (!project_root.isEmpty()) {
            paths.prepend(QDir(project_root).filePath(QStringLiteral("source/assets/icons/%1").arg(name)));
            paths.prepend(QDir(project_root).filePath(QStringLiteral("mocks/icons/%1").arg(name)));
        }

        for (const QString &candidate : paths) {
            if (QFileInfo::exists(candidate)) {
                return QFileInfo(candidate).absoluteFilePath();
            }
        }
    }
#else
    Q_UNUSED(project_root);
    Q_UNUSED(preferred_name);
#endif

    return {};
}

QString find_mock_taskbar_icon_path(const QString &project_root)
{
    return find_mock_icon_path(project_root);
}

void apply_mock_taskbar_icon(QWidget *window, const QString &project_root)
{
    const QString icon_path = find_mock_taskbar_icon_path(project_root);
    if (icon_path.isEmpty()) {
        return;
    }

    const QIcon icon(icon_path);
    QApplication::setWindowIcon(icon);
    if (window != nullptr) {
        window->setWindowIcon(icon);
    }
}

}  // namespace

namespace mocks::translations {

TranslationCompileDialog::TranslationCompileDialog(QWidget *parent, const QString &initial_target_id)
    : QDialog(parent),
      targets_(available_targets())
{
    project_root_ = locate_project_root();
    lrelease_executable_ = locate_lrelease_executable();
    setup_ui();
    populate_targets(initial_target_id);
    sync_path_fields_from_target();
    refresh_context_labels();
    apply_mock_taskbar_icon(this, project_root_);
}

void TranslationCompileDialog::setup_ui()
{
    setWindowTitle(QStringLiteral("Compilador de Traduções - Munin App"));
    resize(900, 640);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    status_label_ = new QLabel(
        QStringLiteral("Pronto. Selecione o módulo e clique em Compilar Traduções."),
        this);
    status_label_->setWordWrap(true);
    layout->addWidget(status_label_);

    auto *target_label = new QLabel(QStringLiteral("Módulo selecionado:"), this);
    layout->addWidget(target_label);

    target_combo_ = new QComboBox(this);
    layout->addWidget(target_combo_);

    auto *source_label = new QLabel(QStringLiteral("Pasta de origem dos arquivos .ts:"), this);
    layout->addWidget(source_label);

    auto *source_row = new QHBoxLayout();
    source_dir_edit_ = new QLineEdit(this);
    source_dir_edit_->setPlaceholderText(QStringLiteral("Use o caminho padrão do módulo selecionado"));
    auto *browse_source_button = new QPushButton(QStringLiteral("Procurar..."), this);
    auto *reset_source_button = new QPushButton(QStringLiteral("Padrão"), this);
    source_row->addWidget(source_dir_edit_, 1);
    source_row->addWidget(browse_source_button);
    source_row->addWidget(reset_source_button);
    layout->addLayout(source_row);

    auto *output_label = new QLabel(QStringLiteral("Pasta de destino dos arquivos .qm:"), this);
    layout->addWidget(output_label);

    auto *output_row = new QHBoxLayout();
    output_dir_edit_ = new QLineEdit(this);
    output_dir_edit_->setPlaceholderText(QStringLiteral("Use a mesma pasta dos arquivos .ts"));
    auto *browse_output_button = new QPushButton(QStringLiteral("Escolher..."), this);
    auto *reset_output_button = new QPushButton(QStringLiteral("Mesmo local"), this);
    output_row->addWidget(output_dir_edit_, 1);
    output_row->addWidget(browse_output_button);
    output_row->addWidget(reset_output_button);
    layout->addLayout(output_row);

    translations_dir_label_ = new QLabel(this);
    translations_dir_label_->setWordWrap(true);
    layout->addWidget(translations_dir_label_);

    lrelease_label_ = new QLabel(this);
    lrelease_label_->setWordWrap(true);
    layout->addWidget(lrelease_label_);

    log_output_ = new QPlainTextEdit(this);
    log_output_->setReadOnly(true);
    log_output_->setLineWrapMode(QPlainTextEdit::NoWrap);
    layout->addWidget(log_output_, 1);

    auto *buttons = new QDialogButtonBox(Qt::Horizontal, this);
    compile_button_ = buttons->addButton(QStringLiteral("Compilar Traduções"), QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Close);
    layout->addWidget(buttons);

    const QString compile_icon_path = find_mock_icon_path(project_root_, QStringLiteral("munin.png"));
    if (!compile_icon_path.isEmpty()) {
        compile_button_->setIcon(QIcon(compile_icon_path));
    }
    const QString browse_icon_path = find_mock_icon_path(project_root_, QStringLiteral("munin_150-150.png"));
    if (!browse_icon_path.isEmpty()) {
        const QIcon browse_icon(browse_icon_path);
        browse_source_button->setIcon(browse_icon);
        browse_output_button->setIcon(browse_icon);
    }

    QObject::connect(compile_button_, &QPushButton::clicked, this, [this] { run_compilation(); });
    QObject::connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    QObject::connect(target_combo_, &QComboBox::currentIndexChanged, this, [this] {
        sync_path_fields_from_target();
        refresh_context_labels();
    });
    QObject::connect(source_dir_edit_, &QLineEdit::textEdited, this, [this] {
        source_dir_custom_ = !clean_path_from_ui(source_dir_edit_->text()).isEmpty();
        if (!output_dir_custom_) {
            sync_path_fields_from_target();
        }
        refresh_context_labels();
    });
    QObject::connect(output_dir_edit_, &QLineEdit::textEdited, this, [this] {
        output_dir_custom_ = !clean_path_from_ui(output_dir_edit_->text()).isEmpty();
        refresh_context_labels();
    });
    QObject::connect(browse_source_button, &QPushButton::clicked, this, [this] { browse_source_directory(); });
    QObject::connect(browse_output_button, &QPushButton::clicked, this, [this] { browse_output_directory(); });
    QObject::connect(reset_source_button, &QPushButton::clicked, this, [this] { reset_source_directory(); });
    QObject::connect(reset_output_button, &QPushButton::clicked, this, [this] { reset_output_directory(); });
}

void TranslationCompileDialog::populate_targets(const QString &initial_target_id)
{
    target_combo_->clear();
    for (const TranslationTarget &target : targets_) {
        target_combo_->addItem(target.display_name, target.id);
    }

    const QString normalized = normalize_target_id(initial_target_id);
    const int requested_index = target_combo_->findData(normalized);
    target_combo_->setCurrentIndex(requested_index >= 0 ? requested_index : 0);
}

void TranslationCompileDialog::append_log(const QString &message)
{
    const QString timestamp =
        QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    log_output_->appendPlainText(QStringLiteral("[%1] %2").arg(timestamp, message));
    QTextCursor cursor = log_output_->textCursor();
    cursor.movePosition(QTextCursor::End);
    log_output_->setTextCursor(cursor);
}

QString TranslationCompileDialog::locate_project_root() const
{
    const QStringList candidates = {
        QCoreApplication::applicationDirPath(),
        QDir::currentPath(),
        QFileInfo(QString::fromLocal8Bit(__FILE__)).absolutePath(),
    };

    for (const QString &candidate : candidates) {
        const QString root = find_project_root_from(candidate);
        if (!root.isEmpty()) {
            return root;
        }
    }

    return {};
}

QString TranslationCompileDialog::locate_lrelease_executable() const
{
    const QString qt_bin = QLibraryInfo::path(QLibraryInfo::BinariesPath);
    const QStringList candidates = {
        QDir(qt_bin).filePath(QStringLiteral("lrelease.exe")),
        QDir(qt_bin).filePath(QStringLiteral("lrelease")),
        QStandardPaths::findExecutable(QStringLiteral("lrelease.exe")),
        QStandardPaths::findExecutable(QStringLiteral("lrelease")),
        QStringLiteral("C:\\Qt\\6.11.1\\mingw_64\\bin\\lrelease.exe"),
    };

    for (const QString &candidate : candidates) {
        if (!candidate.isEmpty() && QFileInfo::exists(candidate)) {
            return QDir::toNativeSeparators(candidate);
        }
    }

    return {};
}

QString TranslationCompileDialog::current_translations_directory() const
{
    if (project_root_.isEmpty()) {
        return {};
    }

    return QDir(project_root_).filePath(current_target().translations_relative_path);
}

QString TranslationCompileDialog::effective_source_directory() const
{
    const QString custom_path =
        source_dir_edit_ != nullptr ? clean_path_from_ui(source_dir_edit_->text()) : QString();
    return custom_path.isEmpty() ? current_translations_directory() : custom_path;
}

QString TranslationCompileDialog::effective_output_directory() const
{
    const QString custom_path =
        output_dir_edit_ != nullptr ? clean_path_from_ui(output_dir_edit_->text()) : QString();
    return custom_path.isEmpty() ? effective_source_directory() : custom_path;
}

TranslationTarget TranslationCompileDialog::current_target() const
{
    const QString current_id = target_combo_->currentData().toString();
    for (const TranslationTarget &target : targets_) {
        if (target.id == current_id) {
            return target;
        }
    }

    return targets_.isEmpty() ? TranslationTarget{} : targets_.constFirst();
}

void TranslationCompileDialog::refresh_context_labels()
{
    if (project_root_.isEmpty()) {
        project_root_ = locate_project_root();
    }
    if (lrelease_executable_.isEmpty()) {
        lrelease_executable_ = locate_lrelease_executable();
    }

    const QString source_dir = effective_source_directory();
    const QString output_dir = effective_output_directory();
    const QString source_mode = source_dir_custom_ ? QStringLiteral("personalizada") : QStringLiteral("padrão");
    const QString output_mode = output_dir_custom_ ? QStringLiteral("personalizada") : QStringLiteral("mesmo local da origem");
    const QString lrelease_text =
        lrelease_executable_.isEmpty() ? QStringLiteral("Não localizado")
                                       : QDir::toNativeSeparators(lrelease_executable_);

    translations_dir_label_->setText(QStringLiteral("Origem .ts (%1): %2\nDestino .qm (%3): %4")
                                         .arg(source_mode,
                                              display_path_or_status(source_dir),
                                              output_mode,
                                              display_path_or_status(output_dir)));
    lrelease_label_->setText(QStringLiteral("Ferramenta lrelease: %1").arg(lrelease_text));
}

void TranslationCompileDialog::sync_path_fields_from_target()
{
    if (source_dir_edit_ == nullptr || output_dir_edit_ == nullptr) {
        return;
    }

    const QString default_source = current_translations_directory();
    if (!source_dir_custom_) {
        const QSignalBlocker blocker(source_dir_edit_);
        source_dir_edit_->setText(QDir::toNativeSeparators(default_source));
    }

    if (!output_dir_custom_) {
        const QSignalBlocker blocker(output_dir_edit_);
        output_dir_edit_->setText(QDir::toNativeSeparators(effective_source_directory()));
    }
}

void TranslationCompileDialog::browse_source_directory()
{
    const QString start_dir = !effective_source_directory().isEmpty()
                                  ? effective_source_directory()
                              : !project_root_.isEmpty()
                                  ? project_root_
                                  : QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString selected_dir =
        QFileDialog::getExistingDirectory(this,
                                          QStringLiteral("Selecionar pasta com arquivos .ts"),
                                          start_dir,
                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (selected_dir.isEmpty()) {
        return;
    }

    source_dir_custom_ = true;
    source_dir_edit_->setText(QDir::toNativeSeparators(QDir::cleanPath(selected_dir)));
    if (!output_dir_custom_) {
        output_dir_edit_->setText(source_dir_edit_->text());
    }
    refresh_context_labels();
}

void TranslationCompileDialog::browse_output_directory()
{
    const QString start_dir = !effective_output_directory().isEmpty()
                                  ? effective_output_directory()
                              : !effective_source_directory().isEmpty()
                                  ? effective_source_directory()
                                  : QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString selected_dir =
        QFileDialog::getExistingDirectory(this,
                                          QStringLiteral("Selecionar pasta de destino dos arquivos .qm"),
                                          start_dir,
                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (selected_dir.isEmpty()) {
        return;
    }

    output_dir_custom_ = true;
    output_dir_edit_->setText(QDir::toNativeSeparators(QDir::cleanPath(selected_dir)));
    refresh_context_labels();
}

void TranslationCompileDialog::reset_source_directory()
{
    source_dir_custom_ = false;
    sync_path_fields_from_target();
    refresh_context_labels();
}

void TranslationCompileDialog::reset_output_directory()
{
    output_dir_custom_ = false;
    sync_path_fields_from_target();
    refresh_context_labels();
}

TranslationCompileDialog::CompileSummary TranslationCompileDialog::compile_current_target(
    const QString &source_directory,
    const QString &output_directory)
{
    CompileSummary summary;
    const QString source_directory_path = clean_path_from_ui(source_directory);
    const QString output_directory_path = clean_path_from_ui(output_directory);
    const QDir source_dir(source_directory_path);
    const QDir output_dir(output_directory_path);
    const QStringList patterns = current_target().file_patterns.isEmpty()
                                     ? QStringList{QStringLiteral("*.ts")}
                                     : current_target().file_patterns;
    const QFileInfoList ts_files =
        source_dir.entryInfoList(patterns, QDir::Files, QDir::Name | QDir::IgnoreCase);

    summary.discovered_files = ts_files.size();
    status_label_->setText(QStringLiteral("Compilando %1 arquivo(s) de tradução de %2...")
                               .arg(summary.discovered_files)
                               .arg(current_target().display_name));

    for (const QFileInfo &ts_file : ts_files) {
        const QString qm_path =
            output_dir.filePath(ts_file.completeBaseName() + QStringLiteral(".qm"));
        append_log(QStringLiteral("Compilando: %1 -> %2")
                       .arg(ts_file.fileName(), QDir::toNativeSeparators(qm_path)));

        QProcess process;
        process.setWorkingDirectory(source_directory_path);
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.start(lrelease_executable_,
                      {QDir::toNativeSeparators(ts_file.absoluteFilePath()),
                       QStringLiteral("-qm"),
                       QDir::toNativeSeparators(qm_path)});

        if (!process.waitForStarted()) {
            ++summary.failed_files;
            append_log(QStringLiteral("Erro ao iniciar o lrelease para %1: %2")
                           .arg(ts_file.fileName(), process.errorString()));
            continue;
        }

        process.waitForFinished(-1);
        const QString output = trim_process_output(QString::fromLocal8Bit(process.readAll()));

        if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
            ++summary.succeeded_files;
            append_log(QStringLiteral("Sucesso: %1").arg(output));
        } else {
            ++summary.failed_files;
            append_log(QStringLiteral("Erro ao compilar %1 (código %2).")
                           .arg(ts_file.fileName())
                           .arg(process.exitCode()));
            append_log(QStringLiteral("Saída: %1").arg(output));
        }

        QApplication::processEvents();
    }

    return summary;
}

void TranslationCompileDialog::run_compilation()
{
    compile_button_->setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool busy_state_released = false;
    const auto release_busy_state = [this, &busy_state_released]() {
        if (busy_state_released) {
            return;
        }

        busy_state_released = true;
        compile_button_->setEnabled(true);
        while (QApplication::overrideCursor() != nullptr) {
            QApplication::restoreOverrideCursor();
        }
    };
    const auto busy_guard = qScopeGuard([&release_busy_state] { release_busy_state(); });

    log_output_->clear();
    append_log(QStringLiteral("Início da compilação das traduções de %1.").arg(current_target().display_name));

    project_root_ = locate_project_root();
    lrelease_executable_ = locate_lrelease_executable();
    sync_path_fields_from_target();
    refresh_context_labels();

    const QString source_dir = effective_source_directory();
    const QString output_dir = effective_output_directory();

    if (!source_dir_custom_ && project_root_.isEmpty()) {
        status_label_->setText(QStringLiteral("Falha: não foi possível localizar a raiz do projeto."));
        append_log(QStringLiteral("Falha ao localizar a raiz do projeto."));
        release_busy_state();
        QMessageBox::critical(this, windowTitle(), QStringLiteral("Não foi possível localizar a raiz do projeto."));
        return;
    }

    if (source_dir.isEmpty() || !QDir(source_dir).exists()) {
        status_label_->setText(QStringLiteral("Falha: diretório de traduções não encontrado."));
        append_log(QStringLiteral("Diretório de origem inexistente: %1")
                       .arg(display_path_or_status(source_dir)));
        release_busy_state();
        QMessageBox::critical(this, windowTitle(), QStringLiteral("Diretório de origem dos arquivos .ts não encontrado."));
        return;
    }

    const QStringList patterns = current_target().file_patterns.isEmpty()
                                     ? QStringList{QStringLiteral("*.ts")}
                                     : current_target().file_patterns;
    const QFileInfoList ts_files =
        QDir(source_dir).entryInfoList(patterns, QDir::Files, QDir::Name | QDir::IgnoreCase);
    if (ts_files.isEmpty()) {
        status_label_->setText(QStringLiteral("Nenhum arquivo .ts foi encontrado."));
        append_log(QStringLiteral("Nenhum arquivo .ts foi encontrado em: %1")
                       .arg(QDir::toNativeSeparators(source_dir)));
        release_busy_state();
        QMessageBox::information(this, windowTitle(), QStringLiteral("Nenhum arquivo .ts foi encontrado."));
        return;
    }

    if (lrelease_executable_.isEmpty()) {
        status_label_->setText(QStringLiteral("Falha: lrelease.exe não encontrado."));
        append_log(QStringLiteral("Não foi possível localizar o executável lrelease.exe."));
        release_busy_state();
        QMessageBox::critical(this, windowTitle(), QStringLiteral("Não foi possível localizar o lrelease.exe do Qt."));
        return;
    }

    if (output_dir.isEmpty()) {
        status_label_->setText(QStringLiteral("Falha: diretório de destino não informado."));
        append_log(QStringLiteral("Diretório de destino dos arquivos .qm não informado."));
        release_busy_state();
        QMessageBox::critical(this, windowTitle(), QStringLiteral("Informe o diretório de destino dos arquivos .qm."));
        return;
    }

    if (!QDir(output_dir).exists()) {
        const QMessageBox::StandardButton answer =
            QMessageBox::question(this,
                                  windowTitle(),
                                  QStringLiteral("O diretório de destino não existe.\n\n%1\n\nDeseja criá-lo agora?")
                                      .arg(QDir::toNativeSeparators(output_dir)),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);
        if (answer != QMessageBox::Yes) {
            status_label_->setText(QStringLiteral("Compilação cancelada."));
            append_log(QStringLiteral("Compilação cancelada: diretório de destino inexistente."));
            release_busy_state();
            return;
        }

        if (!QDir().mkpath(output_dir)) {
            status_label_->setText(QStringLiteral("Falha: não foi possível criar o diretório de destino."));
            append_log(QStringLiteral("Falha ao criar diretório de destino: %1")
                           .arg(QDir::toNativeSeparators(output_dir)));
            release_busy_state();
            QMessageBox::critical(this,
                                  windowTitle(),
                                  QStringLiteral("Não foi possível criar o diretório de destino dos arquivos .qm."));
            return;
        }
        append_log(QStringLiteral("Diretório de destino criado: %1").arg(QDir::toNativeSeparators(output_dir)));
        refresh_context_labels();
    }

    append_log(QStringLiteral("Origem .ts: %1").arg(QDir::toNativeSeparators(source_dir)));
    append_log(QStringLiteral("Destino .qm: %1").arg(QDir::toNativeSeparators(output_dir)));

    const CompileSummary summary = compile_current_target(source_dir, output_dir);

    const QString summary_text =
        QStringLiteral("Arquivos encontrados: %1 | Sucesso: %2 | Falhas: %3")
            .arg(summary.discovered_files)
            .arg(summary.succeeded_files)
            .arg(summary.failed_files);

    status_label_->setText(summary.has_failures()
                                ? QStringLiteral("Compilação finalizada com falhas. %1").arg(summary_text)
                                : QStringLiteral("Compilação finalizada com sucesso. %1").arg(summary_text));
    append_log(summary_text);

    release_busy_state();
    if (summary.has_failures()) {
        QMessageBox::warning(this, windowTitle(),
                             QStringLiteral("A compilação terminou com falhas.\n\n%1").arg(summary_text));
    } else {
        QMessageBox::information(this, windowTitle(),
                                 QStringLiteral("Todas as traduções foram compiladas.\n\n%1").arg(summary_text));
    }
}

}  // namespace mocks::translations

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    mocks::translations::TranslationCompileDialog dialog(nullptr, argument_target_id(app.arguments()));
    dialog.show();
    return app.exec();
}
