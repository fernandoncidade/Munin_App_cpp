#include "find_VersionEditor.hpp"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMap>
#include <QPair>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>
#include <QtGlobal>

#include <optional>

#if defined(Q_OS_WIN)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace mocks::version_editor {
namespace {

const QStringList kPtMonths = {
    QStringLiteral("Janeiro"),
    QStringLiteral("Fevereiro"),
    QStringLiteral("Março"),
    QStringLiteral("Abril"),
    QStringLiteral("Maio"),
    QStringLiteral("Junho"),
    QStringLiteral("Julho"),
    QStringLiteral("Agosto"),
    QStringLiteral("Setembro"),
    QStringLiteral("Outubro"),
    QStringLiteral("Novembro"),
    QStringLiteral("Dezembro"),
};

const QStringList kPtMonthsLower = {
    QStringLiteral("janeiro"),
    QStringLiteral("fevereiro"),
    QStringLiteral("março"),
    QStringLiteral("abril"),
    QStringLiteral("maio"),
    QStringLiteral("junho"),
    QStringLiteral("julho"),
    QStringLiteral("agosto"),
    QStringLiteral("setembro"),
    QStringLiteral("outubro"),
    QStringLiteral("novembro"),
    QStringLiteral("dezembro"),
};

const QStringList kEnMonths = {
    QStringLiteral("January"),
    QStringLiteral("February"),
    QStringLiteral("March"),
    QStringLiteral("April"),
    QStringLiteral("May"),
    QStringLiteral("June"),
    QStringLiteral("July"),
    QStringLiteral("August"),
    QStringLiteral("September"),
    QStringLiteral("October"),
    QStringLiteral("November"),
    QStringLiteral("December"),
};

const QString &group_about()
{
    static const QString value = QStringLiteral("Sobre");
    return value;
}

const QString &group_eula()
{
    static const QString value = QStringLiteral("EULA");
    return value;
}

const QString &group_notices()
{
    static const QString value = QStringLiteral("Notices");
    return value;
}

const QString &group_clc()
{
    static const QString value = QStringLiteral("CLC");
    return value;
}

const QString &group_privacy()
{
    static const QString value = QStringLiteral("Política de Privacidade");
    return value;
}

const QString &group_source()
{
    static const QString value = QStringLiteral("Código Fonte (C++)");
    return value;
}

const QString &group_docs()
{
    static const QString value = QStringLiteral("Documentação");
    return value;
}

const QString &group_installers()
{
    static const QString value = QStringLiteral("Instaladores (Inno Setup)");
    return value;
}

const QStringList &group_order()
{
    static const QStringList order = {
        group_about(),
        group_clc(),
        group_eula(),
        group_notices(),
        group_privacy(),
        group_source(),
        group_docs(),
        group_installers(),
    };
    return order;
}

QString build_version_from_date(const QDate &date = QDate::currentDate())
{
    const QDate safe_date = date.isValid() ? date : QDate::currentDate();
    if (!safe_date.isValid()) {
        return QStringLiteral("2026.10.17.0");
    }

    return QStringLiteral("%1.%2.%3.0")
        .arg(safe_date.year())
        .arg(safe_date.month())
        .arg(safe_date.day());
}

QString readme_version_token(const QString &version)
{
    const QString trimmed = version.trimmed();
    if (trimmed.isEmpty()) {
        return QStringLiteral("v0.0.0.0");
    }

    return trimmed.startsWith(QLatin1Char('v'), Qt::CaseInsensitive) ? trimmed : QStringLiteral("v%1").arg(trimmed);
}

QString spec_content_key(const FileSpec &spec)
{
    return spec.content_key.isEmpty() ? spec.key : spec.content_key;
}

QString find_release_doc_relative_path(const QString &base_dir)
{
    const QString root = base_dir.isEmpty() ? detect_project_root() : base_dir;
    const QDir docs_dir(QDir(root).filePath(QStringLiteral("docs")));
    const QStringList files = docs_dir.entryList({QStringLiteral("release-v*.md")}, QDir::Files, QDir::Name);
    if (!files.isEmpty()) {
        return QStringLiteral("docs/%1").arg(files.first());
    }
    return QStringLiteral("docs/release-v2026.10.17.0.md");
}

const QMap<QString, QStringList> &patterns()
{
    static const QMap<QString, QStringList> data = {
        {QStringLiteral("about_pt"), {QStringLiteral(R"(^(Vers(?:ã|a)o:\s*[\d\.]+))")}},
        {QStringLiteral("about_en"), {QStringLiteral(R"(^(Version:\s*[\d\.]+))")}},
        {QStringLiteral("clc_pt"),
         {QStringLiteral(R"(^(Vers(?:ã|a)o\s+[\d\.]+,?\s+.*)$)"),
          QStringLiteral(R"((Brasil,\s+Paran(?:á|a),\s+S(?:ã|a)o Jos(?:é|e) dos Pinhais,\s+.*\.))")}},
        {QStringLiteral("clc_en"),
         {QStringLiteral(R"(^(Version\s+[\d\.]+,?\s+.*)$)"),
          QStringLiteral(R"((Brazil,\s+Paran(?:á|a),\s+S(?:ã|a)o Jos(?:é|e) dos Pinhais,\s+.*\.))")}},
        {QStringLiteral("eula_pt"), {QStringLiteral(R"(^(Versão\s+[\d\.]+,?\s+.*)$)")}},
        {QStringLiteral("eula_en"), {QStringLiteral(R"(^(Version\s+[\d\.]+,?\s+.*)$)")}},
        {QStringLiteral("notice_pt"), {QStringLiteral(R"(^(Versão\s+[\d\.]+,?\s+.*)$)")}},
        {QStringLiteral("notice_en"), {QStringLiteral(R"(^(Version\s+[\d\.]+,?\s+.*)$)")}},
        {QStringLiteral("privacy_pt"),
         {QStringLiteral(R"(^(Última atualiza(?:çã|ca)o:\s*.*)$)"),
          QStringLiteral(R"(^(Vers(?:ã|a)o:\s*[\d\.]+))")}},
        {QStringLiteral("privacy_en"),
         {QStringLiteral(R"(^(Last updated:\s*.*)$)"),
          QStringLiteral(R"(^(Version:\s*[\d\.]+))")}},

        {QStringLiteral("main_root"),
         {QStringLiteral(R"((app\.setApplicationVersion\(QStringLiteral\("[\d\.]+"\)\);))")}},
        {QStringLiteral("main_source"),
         {QStringLiteral(R"((app\.setApplicationVersion\(QStringLiteral\("[\d\.]+"\)\);))"),
          QStringLiteral(R"((=== Iniciando Munin App v[\d\.]+ ===))")}},
        {QStringLiteral("main_cli"),
         {QStringLiteral(R"((static constexpr const char\*\s+APP_VERSION\s*=\s*"[\d\.]+";))")}},
        {QStringLiteral("about_dialog"),
         {QStringLiteral(R"((<b>Vers(?:ã|a)o:</b>\s*[\d\.]+))"),
          QStringLiteral(R"((<b>Version:</b>\s*[\d\.]+))")}},
        {QStringLiteral("dialog_manager"),
         {QStringLiteral(R"((<b>Vers(?:ã|a)o:</b>\s*[\d\.]+))"),
          QStringLiteral(R"((<b>Version:</b>\s*[\d\.]+))")}},

        {QStringLiteral("readme"),
         {QStringLiteral(R"((\[!\[(?:Latest Release|Official Version|Vers(?:ã|a)o Oficial)\]\(https://img\.shields\.io/badge/(?:Release-v?|Version-|Vers(?:ã|a)o-)[\d\.]+-blue\.svg\)))"),
          QStringLiteral(R"((Version:\s*v?[\d\.]+<br>))"),
          QStringLiteral(R"((Technical revision date:\s*.*<br>))"),
          QStringLiteral(R"((Vers(?:ã|a)o:\s*v?[\d\.]+<br>))"),
          QStringLiteral(R"((Data t(?:é|e)cnica desta revis(?:ã|a)o:\s*.*<br>))")}},
        {QStringLiteral("manual"),
         {QStringLiteral(R"(((?:\*\*Vers(?:ã|a)o:\*\*|Vers(?:ã|a)o documentada:)\s*`[\d\.]+`))"),
          QStringLiteral(R"(((?:\*\*Data:\*\*|Data:)\s*`.*`))"),
          QStringLiteral(R"(((?:\*\*Version:\*\*|Version documented:|Documented version:)\s*`[\d\.]+`))"),
          QStringLiteral(R"(((?:\*\*Date:\*\*|Date:)\s*`.*`))")}},
        {QStringLiteral("observacao"),
         {QStringLiteral(R"((Vers(?:ã|a)o documentada:\s*`[\d\.]+`))"),
          QStringLiteral(R"((Data do snapshot t(?:é|e)cnico:\s*`.*`))"),
          QStringLiteral(R"((## 2\. Linha T(?:é|e)cnica e Destaques da Vers(?:ã|a)o\s+[\d\.]+))")}},
        {QStringLiteral("release_doc"),
         {QStringLiteral(R"((\*\*Tag de Vers(?:ã|a)o:\*\*\s*`v?[\d\.]+`))"),
          QStringLiteral(R"((\*\*Data de Publica(?:çã|ca)o:\*\*\s*.*))"),
          QStringLiteral(R"((\*\*Release Tag:\*\*\s*`v?[\d\.]+`))"),
          QStringLiteral(R"((\*\*Publication Date:\*\*\s*.*))")}},

        {QStringLiteral("installer_unified"),
         {QStringLiteral(R"((#define\s+MyAppVersion\s+"[\d\.]+"))")}},
        {QStringLiteral("installer_mingw"),
         {QStringLiteral(R"((#define\s+MyAppVersion\s+"[\d\.]+"))"),
          QStringLiteral(R"((OutputBaseFilename=Munin_mingw_v[\d\.]+))")}},
        {QStringLiteral("installer_msvc"),
         {QStringLiteral(R"((#define\s+MyAppVersion\s+"[\d\.]+"))"),
          QStringLiteral(R"((OutputBaseFilename=Munin_msvc_v[\d\.]+))")}},
    };

    return data;
}

QString resolve_file_path(const QString &base_dir, const QString &relative_path)
{
    const QDir dir(base_dir);
    const QString primary = QFileInfo(dir.filePath(relative_path)).absoluteFilePath();
    if (QFileInfo::exists(primary)) {
        return primary;
    }

    const QStringList rel_parts = QDir::fromNativeSeparators(relative_path).split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (!rel_parts.isEmpty() && rel_parts.first() == QStringLiteral("source")) {
        QString stripped;
        for (int i = 1; i < rel_parts.size(); ++i) {
            if (!stripped.isEmpty()) {
                stripped += QLatin1Char('/');
            }
            stripped += rel_parts.at(i);
        }

        const QString alternate = QFileInfo(dir.filePath(stripped)).absoluteFilePath();
        if (QFileInfo::exists(alternate)) {
            return alternate;
        }
    }

    return primary;
}

std::optional<QString> read_file_text(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return std::nullopt;
    }

    QByteArray data = file.readAll();
    if (data.startsWith("\xEF\xBB\xBF")) {
        data = data.mid(3);
    }

    QString utf8 = QString::fromUtf8(data);
    if (!utf8.contains(QChar::ReplacementCharacter)) {
        return utf8;
    }

    QString latin1 = QString::fromLatin1(data);
    return latin1;
}

bool write_file_text(const QString &path, const QString &text, QString *error_message)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    const qint64 written = file.write(text.toUtf8());
    if (written < 0) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    return true;
}

QString detect_line_ending(const QString &text)
{
    return text.contains(QStringLiteral("\r\n")) ? QStringLiteral("\r\n") : QStringLiteral("\n");
}

QStringList split_lines_preserving_empty(const QString &text)
{
    return text.split(QRegularExpression(QStringLiteral(R"(\r\n|\n|\r)")), Qt::KeepEmptyParts);
}

QString join_lines(const QStringList &lines, const QString &separator)
{
    return lines.join(separator);
}

QStringList first_lines(const QString &text, int limit)
{
    QStringList lines = split_lines_preserving_empty(text);
    if (lines.size() > limit) {
        lines = lines.mid(0, limit);
    }
    return lines;
}

QString replace_readme_version_token(const QString &line, const QString &version)
{
    const QRegularExpression rx(QStringLiteral(R"(^(.*\*\*)v[\d\.]+(\*\*.*)$)"));
    const QRegularExpressionMatch match = rx.match(line);
    if (!match.hasMatch()) {
        return line;
    }

    return QStringLiteral("%1%2%3").arg(match.captured(1), version, match.captured(2));
}

QString mock_windows_icon_name()
{
#if defined(MOCKS_WINDOWS_ICON_NAME)
    return QStringLiteral(MOCKS_WINDOWS_ICON_NAME);
#else
    return QStringLiteral("munin.ico");
#endif
}

QString find_icon_path(const QString &project_root, const QString &preferred_name = QString())
{
    const QString primary_icon_name = preferred_name.trimmed().isEmpty() ? mock_windows_icon_name() : preferred_name.trimmed();
    const QStringList candidate_names = {
        primary_icon_name,
        QStringLiteral("munin.ico"),
        QStringLiteral("munin.png"),
        QStringLiteral("munin_300-300.png"),
        QStringLiteral("munin_150-150.png")
    };

    const QDir root(project_root);
    const QDir app_dir(QCoreApplication::applicationDirPath());
    const QDir current_dir(QDir::currentPath());
    const QDir source_dir(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());

    for (const QString &icon_name : candidate_names) {
        if (icon_name.isEmpty()) {
            continue;
        }
        QStringList candidates = {
            app_dir.filePath(QStringLiteral("icons/%1").arg(icon_name)),
            app_dir.filePath(icon_name),
            current_dir.filePath(QStringLiteral("icons/%1").arg(icon_name)),
            current_dir.filePath(QStringLiteral("mocks/icons/%1").arg(icon_name)),
            source_dir.filePath(QStringLiteral("icons/%1").arg(icon_name)),
            source_dir.filePath(QStringLiteral("../source/assets/icons/%1").arg(icon_name)),
        };

        if (!project_root.isEmpty()) {
            candidates.prepend(root.filePath(QStringLiteral("source/assets/icons/%1").arg(icon_name)));
            candidates.prepend(root.filePath(QStringLiteral("mocks/icons/%1").arg(icon_name)));
            candidates.prepend(root.filePath(QStringLiteral("icons/%1").arg(icon_name)));
        }

        for (const QString &candidate : candidates) {
            if (QFileInfo::exists(candidate)) {
                return QFileInfo(candidate).absoluteFilePath();
            }
        }
    }

    return QString();
}

}  // namespace

QDate parse_date_from_version(const QString &version)
{
    const QString trimmed = version.trimmed().startsWith(QLatin1Char('v'), Qt::CaseInsensitive)
                                ? version.trimmed().mid(1)
                                : version.trimmed();
    const QStringList parts = trimmed.split(QLatin1Char('.'));
    if (parts.size() >= 3) {
        bool ok_y = false, ok_m = false, ok_d = false;
        const int y = parts[0].toInt(&ok_y);
        const int m = parts[1].toInt(&ok_m);
        const int d = parts[2].toInt(&ok_d);
        if (ok_y && ok_m && ok_d) {
            const QDate dt(y, m, d);
            if (dt.isValid()) {
                return dt;
            }
        }
    }
    return QDate::currentDate();
}

QString normalize_dir(const QString &path)
{
    if (path.trimmed().isEmpty()) {
        return QString();
    }

    return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

QString detect_project_root()
{
    const QString app_dir = QCoreApplication::applicationDirPath();
    const QString current_dir = QDir::currentPath();
    const QString source_file_dir = QFileInfo(QString::fromUtf8(__FILE__)).absolutePath();

    QStringList seeds = {app_dir, current_dir, source_file_dir};
    QStringList checked;

    auto is_project_root = [](const QString &candidate) {
        const QDir dir(candidate);
        return dir.exists(QStringLiteral("mocks")) && dir.exists(QStringLiteral("source"))
               && QFileInfo(dir.filePath(QStringLiteral("README.md"))).exists();
    };

    for (const QString &seed : seeds) {
        QDir dir(seed);
        for (int depth = 0; depth < 8; ++depth) {
            const QString candidate = normalize_dir(dir.absolutePath());
            if (!checked.contains(candidate)) {
                checked.append(candidate);
                if (is_project_root(candidate)) {
                    return candidate;
                }
            }

            if (!dir.cdUp()) {
                break;
            }
        }
    }

    return normalize_dir(QDir(source_file_dir).filePath(QStringLiteral("..")));
}

QList<FileSpec> file_specs(const QString &base_dir)
{
    QList<FileSpec> data;

    auto make_key = [](const QString &relative_path) {
        QString key;
        bool last_was_separator = false;
        for (const QChar ch : relative_path.toCaseFolded()) {
            if (ch.isLetterOrNumber()) {
                key.append(ch);
                last_was_separator = false;
            } else if (!last_was_separator) {
                key.append(QLatin1Char('_'));
                last_was_separator = true;
            }
        }

        while (key.endsWith(QLatin1Char('_'))) {
            key.chop(1);
        }
        return key;
    };

    auto append = [&](const QString &group,
                      const QString &relative_path,
                      const QString &content_key) {
        data.append({make_key(relative_path), relative_path, content_key, group});
    };

    // 1. Arquivos Sobre (ABOUT)
    append(group_about(), QStringLiteral("source/assets/ABOUT/ABOUT_pt_BR.txt"), QStringLiteral("about_pt"));
    append(group_about(), QStringLiteral("source/assets/ABOUT/ABOUT_en_US.txt"), QStringLiteral("about_en"));

    // 2. Contratos Comerciais (CLC)
    append(group_clc(), QStringLiteral("source/assets/CLC/CLC_pt_BR.txt"), QStringLiteral("clc_pt"));
    append(group_clc(), QStringLiteral("source/assets/CLC/CLC_en_US.txt"), QStringLiteral("clc_en"));

    // 3. Licenças EULA
    append(group_eula(), QStringLiteral("source/assets/EULA/EULA_pt_BR.txt"), QStringLiteral("eula_pt"));
    append(group_eula(), QStringLiteral("source/assets/EULA/EULA_en_US.txt"), QStringLiteral("eula_en"));

    // 4. Notices & Terceiros
    append(group_notices(), QStringLiteral("source/assets/NOTICES/NOTICE_pt_BR.txt"), QStringLiteral("notice_pt"));
    append(group_notices(), QStringLiteral("source/assets/NOTICES/NOTICE_en_US.txt"), QStringLiteral("notice_en"));

    // 5. Políticas de Privacidade
    append(group_privacy(), QStringLiteral("source/assets/PRIVACY_POLICY/Privacy_Policy_pt_BR.txt"), QStringLiteral("privacy_pt"));
    append(group_privacy(), QStringLiteral("source/assets/PRIVACY_POLICY/Privacy_Policy_en_US.txt"), QStringLiteral("privacy_en"));

    // 6. Código Fonte Principal e Diálogos (C++)
    append(group_source(), QStringLiteral("main.cpp"), QStringLiteral("main_root"));
    append(group_source(), QStringLiteral("source/main.cpp"), QStringLiteral("main_source"));
    append(group_source(), QStringLiteral("source/cli/main_cli.cpp"), QStringLiteral("main_cli"));
    append(group_source(), QStringLiteral("source/ui/SobreDialog.cpp"), QStringLiteral("about_dialog"));
    append(group_source(), QStringLiteral("source/ui/GerenciadorDialogos.cpp"), QStringLiteral("dialog_manager"));

    // 7. Documentação Técnica Completa e Guias Modulares (Markdown)
    append(group_docs(), QStringLiteral("README.md"), QStringLiteral("readme"));
    append(group_docs(), QStringLiteral("MANUAL.md"), QStringLiteral("manual"));
    append(group_docs(), QStringLiteral("OBSERVACAO.md"), QStringLiteral("observacao"));
    append(group_docs(), QStringLiteral("docs/architecture.md"), QStringLiteral("manual"));
    append(group_docs(), QStringLiteral("docs/cli-reference.md"), QStringLiteral("manual"));
    append(group_docs(), QStringLiteral("docs/compliance.md"), QStringLiteral("manual"));
    append(group_docs(), QStringLiteral("docs/licensing.md"), QStringLiteral("manual"));
    append(group_docs(), QStringLiteral("docs/security-model.md"), QStringLiteral("manual"));
    append(group_docs(), QStringLiteral("docs/user-guide.md"), QStringLiteral("manual"));
    append(group_docs(), find_release_doc_relative_path(base_dir), QStringLiteral("release_doc"));

    // 8. Instaladores (Inno Setup)
    append(group_installers(), QStringLiteral("repo/Munin.iss"), QStringLiteral("installer_unified"));
    append(group_installers(), QStringLiteral("repo/Munin_MinGW.iss"), QStringLiteral("installer_mingw"));
    append(group_installers(), QStringLiteral("repo/Munin_MSVC.iss"), QStringLiteral("installer_msvc"));

    return data;
}

const QList<FileSpec> &file_specs()
{
    static const QList<FileSpec> static_specs = file_specs(detect_project_root());
    return static_specs;
}

QList<FileStatus> check_expected_lines(const QString &base_dir)
{
    QList<FileStatus> statuses;

    const auto specs = file_specs(base_dir);
    for (const FileSpec &spec : specs) {
        FileStatus status;
        status.key = spec.key;
        status.group = spec.group.isEmpty() ? group_source() : spec.group;
        status.relative_path = spec.relative_path;
        status.path = resolve_file_path(base_dir, spec.relative_path);

        const std::optional<QString> text = read_file_text(status.path);
        if (!text.has_value()) {
            status.exists = false;
            statuses.append(status);
            continue;
        }

        status.exists = true;
        const QStringList pattern_list = patterns().value(spec_content_key(spec));
        if (pattern_list.isEmpty()) {
            status.found = first_lines(text.value(), 3);
            statuses.append(status);
            continue;
        }

        for (const QString &pattern : pattern_list) {
            const QRegularExpression rx(pattern, QRegularExpression::MultilineOption);
            const QRegularExpressionMatch match = rx.match(text.value());
            if (match.hasMatch()) {
                status.found.append(match.captured(1).trimmed());
            } else {
                status.missing.append(pattern);
            }
        }

        statuses.append(status);
    }

    return statuses;
}

bool apply_line_updates(const QString &key, QStringList *lines, const UpdateContext &context)
{
    if (lines == nullptr) {
        return false;
    }

    bool changed = false;
    for (QString &line : *lines) {
        const QString original = line;

        if (key == QStringLiteral("readme") || key == QStringLiteral("readme_pt") || key == QStringLiteral("readme_en")) {
            if (line.contains(QStringLiteral("img.shields.io/badge/Release-"))) {
                const QRegularExpression rx_badge_rel(
                    QStringLiteral(R"((https://img\.shields\.io/badge/Release-v?)[\d\.]+(-blue\.svg))"));
                const QRegularExpressionMatch match_badge_rel = rx_badge_rel.match(line);
                if (match_badge_rel.hasMatch()) {
                    line = line.left(match_badge_rel.capturedStart(0)) + match_badge_rel.captured(1)
                           + context.pt_version + match_badge_rel.captured(2)
                           + line.mid(match_badge_rel.capturedEnd(0));
                }
            }

            if (line.contains(QStringLiteral("img.shields.io/badge/Versão-"))
                || line.contains(QStringLiteral("img.shields.io/badge/Versao-"))) {
                const QRegularExpression rx_badge_pt(
                    QStringLiteral(R"((https://img\.shields\.io/badge/Vers(?:ã|a)o-)[\d\.]+(-blue\.svg))"));
                const QRegularExpressionMatch match_badge_pt = rx_badge_pt.match(line);
                if (match_badge_pt.hasMatch()) {
                    line = line.left(match_badge_pt.capturedStart(0)) + match_badge_pt.captured(1)
                           + context.pt_version + match_badge_pt.captured(2)
                           + line.mid(match_badge_pt.capturedEnd(0));
                }
            }

            if (line.contains(QStringLiteral("img.shields.io/badge/Version-"))) {
                const QRegularExpression rx_badge_en(
                    QStringLiteral(R"((https://img\.shields\.io/badge/Version-)[\d\.]+(-blue\.svg))"));
                const QRegularExpressionMatch match_badge_en = rx_badge_en.match(line);
                if (match_badge_en.hasMatch()) {
                    line = line.left(match_badge_en.capturedStart(0)) + match_badge_en.captured(1)
                           + context.en_version + match_badge_en.captured(2)
                           + line.mid(match_badge_en.capturedEnd(0));
                }
            }

            // - 📦 [Release Notes v2026.10.17.0](docs/release-v2026.10.17.0.md)
            const QRegularExpression rx_readme_rel_en(
                QStringLiteral(R"((- 📦 \[Release Notes v)[\d\.]+(\]\(docs/release-v)[\d\.]+(\.md\)))"));
            const QRegularExpressionMatch match_readme_rel_en = rx_readme_rel_en.match(line);
            if (match_readme_rel_en.hasMatch()) {
                line = match_readme_rel_en.captured(1) + context.en_version
                       + match_readme_rel_en.captured(2) + context.en_version
                       + match_readme_rel_en.captured(3)
                       + line.mid(match_readme_rel_en.capturedEnd(0));
            }

            // - 📦 [Notas de Versão v2026.10.17.0](docs/release-v2026.10.17.0.md)
            const QRegularExpression rx_readme_rel_pt(
                QStringLiteral(R"((- 📦 \[Notas de Vers(?:ã|a)o v)[\d\.]+(\]\(docs/release-v)[\d\.]+(\.md\)))"));
            const QRegularExpressionMatch match_readme_rel_pt = rx_readme_rel_pt.match(line);
            if (match_readme_rel_pt.hasMatch()) {
                line = match_readme_rel_pt.captured(1) + context.pt_version
                       + match_readme_rel_pt.captured(2) + context.pt_version
                       + match_readme_rel_pt.captured(3)
                       + line.mid(match_readme_rel_pt.capturedEnd(0));
            }

            if (line.startsWith(QStringLiteral("> **Observação:**")) ||
                line.startsWith(QStringLiteral("> **Observacao:**"))) {
                line = replace_readme_version_token(line, context.pt_readme_version);
            } else if (line.startsWith(QStringLiteral("> **Note:**"))) {
                line = replace_readme_version_token(line, context.en_readme_version);
            } else if (line.startsWith(QStringLiteral("Versão:"))) {
                line = QStringLiteral("Versão: %1<br>").arg(context.pt_readme_version);
            } else if (line.startsWith(QStringLiteral("Versao:"))) {
                line = QStringLiteral("Versao: %1<br>").arg(context.pt_readme_version);
            } else if (line.startsWith(QStringLiteral("Version:"))) {
                line = QStringLiteral("Version: %1<br>").arg(context.en_readme_version);
            } else if (line.startsWith(QStringLiteral("Data técnica desta revisão:"))) {
                line = QStringLiteral("Data técnica desta revisão: %1<br>").arg(context.pt_date);
            } else if (line.startsWith(QStringLiteral("Data tecnica desta revisao:"))) {
                line = QStringLiteral("Data tecnica desta revisao: %1<br>").arg(context.pt_date);
            } else if (line.startsWith(QStringLiteral("Technical revision date:"))) {
                line = QStringLiteral("Technical revision date: %1<br>").arg(context.en_date);
            }
        } else if (key == QStringLiteral("manual")) {
            const QRegularExpression rx_manual_ver(QStringLiteral(R"((\*\*Vers(?:ã|a)o:\*\*\s*`)[^`]+(`))"));
            const QRegularExpressionMatch match_ver = rx_manual_ver.match(line);
            if (match_ver.hasMatch()) {
                line = line.left(match_ver.capturedStart(0)) + match_ver.captured(1)
                       + context.pt_version + match_ver.captured(2)
                       + line.mid(match_ver.capturedEnd(0));
            }

            const QRegularExpression rx_manual_date(QStringLiteral(R"((\*\*Data:\*\*\s*`)[^`]+(`))"));
            const QRegularExpressionMatch match_date = rx_manual_date.match(line);
            if (match_date.hasMatch()) {
                line = line.left(match_date.capturedStart(0)) + match_date.captured(1)
                       + context.pt_date + match_date.captured(2)
                       + line.mid(match_date.capturedEnd(0));
            }

            const QRegularExpression rx_manual_en_ver(QStringLiteral(R"((\*\*Version:\*\*\s*`)[^`]+(`))"));
            const QRegularExpressionMatch match_en_ver = rx_manual_en_ver.match(line);
            if (match_en_ver.hasMatch()) {
                line = line.left(match_en_ver.capturedStart(0)) + match_en_ver.captured(1)
                       + context.en_version + match_en_ver.captured(2)
                       + line.mid(match_en_ver.capturedEnd(0));
            }

            const QRegularExpression rx_manual_en_date(QStringLiteral(R"((\*\*Date:\*\*\s*`)[^`]+(`))"));
            const QRegularExpressionMatch match_en_date = rx_manual_en_date.match(line);
            if (match_en_date.hasMatch()) {
                line = line.left(match_en_date.capturedStart(0)) + match_en_date.captured(1)
                       + context.en_date + match_en_date.captured(2)
                       + line.mid(match_en_date.capturedEnd(0));
            }

            if (line.startsWith(QStringLiteral("Versão documentada:"))
                || line.startsWith(QStringLiteral("Versao documentada:"))) {
                line = line.startsWith(QStringLiteral("Versao"))
                           ? QStringLiteral("Versao documentada: `%1`  ").arg(context.pt_version)
                           : QStringLiteral("Versão documentada: `%1`  ").arg(context.pt_version);
            } else if (line.startsWith(QStringLiteral("Version documented:"))) {
                line = QStringLiteral("Version documented: `%1`  ").arg(context.en_version);
            } else if (line.startsWith(QStringLiteral("Documented version:"))) {
                line = QStringLiteral("Documented version: `%1`  ").arg(context.en_version);
            } else if (line.startsWith(QStringLiteral("Data:")) || line.startsWith(QStringLiteral("Data :"))) {
                line = QStringLiteral("Data: `%1`  ").arg(context.pt_date);
            } else if (line.startsWith(QStringLiteral("Date:")) || line.startsWith(QStringLiteral("Date :"))) {
                line = QStringLiteral("Date: `%1`  ").arg(context.en_date);
            }
        } else if (key == QStringLiteral("observacao") || key == QStringLiteral("observacao_pt")) {
            if (line.startsWith(QStringLiteral("Versão documentada:"))
                || line.startsWith(QStringLiteral("Versao documentada:"))) {
                line = line.startsWith(QStringLiteral("Versao"))
                           ? QStringLiteral("Versao documentada: `%1`  ").arg(context.pt_version)
                           : QStringLiteral("Versão documentada: `%1`  ").arg(context.pt_version);
            } else if (line.startsWith(QStringLiteral("Data do snapshot técnico:"))
                       || line.startsWith(QStringLiteral("Data do snapshot tecnico:"))) {
                line = line.startsWith(QStringLiteral("Data do snapshot tecnico"))
                           ? QStringLiteral("Data do snapshot tecnico: `%1`  ").arg(context.pt_date)
                           : QStringLiteral("Data do snapshot técnico: `%1`  ").arg(context.pt_date);
            } else if (line.startsWith(QStringLiteral("Version documented:"))) {
                line = QStringLiteral("Version documented: `%1`  ").arg(context.en_version);
            } else if (line.startsWith(QStringLiteral("Documented version:"))) {
                line = QStringLiteral("Documented version: `%1`  ").arg(context.en_version);
            } else if (line.startsWith(QStringLiteral("Technical snapshot date:"))) {
                line = QStringLiteral("Technical snapshot date: `%1`  ").arg(context.en_date);
            } else {
                const QRegularExpression rx_heading_pt(
                    QStringLiteral(R"((## 2\. Linha T(?:é|e)cnica e Destaques da Vers(?:ã|a)o\s+)[\d\.]+)"));
                const QRegularExpressionMatch match_heading_pt = rx_heading_pt.match(line);
                if (match_heading_pt.hasMatch()) {
                    line = match_heading_pt.captured(1) + context.pt_version;
                }

                const QRegularExpression rx_heading_en(
                    QStringLiteral(R"((## 2\. Technical Line and Highlights of Version\s+)[\d\.]+)"));
                const QRegularExpressionMatch match_heading_en = rx_heading_en.match(line);
                if (match_heading_en.hasMatch()) {
                    line = match_heading_en.captured(1) + context.en_version;
                }

                const QRegularExpression rx_body_ver(
                    QStringLiteral(R"((^A vers(?:ã|a)o\s+)[\d\.]+(\s+marca o lan(?:ç|c)amento oficial.*)$)"));
                const QRegularExpressionMatch match_body_ver = rx_body_ver.match(line);
                if (match_body_ver.hasMatch()) {
                    line = match_body_ver.captured(1) + context.pt_version + match_body_ver.captured(2);
                }

                const QRegularExpression rx_iss_mingw(
                    QStringLiteral(R"((Munin_mingw_v)[\d\.]+(\.exe))"));
                const QRegularExpressionMatch match_iss_mingw = rx_iss_mingw.match(line);
                if (match_iss_mingw.hasMatch()) {
                    line = line.left(match_iss_mingw.capturedStart(0)) + match_iss_mingw.captured(1)
                           + context.pt_version + match_iss_mingw.captured(2)
                           + line.mid(match_iss_mingw.capturedEnd(0));
                }

                const QRegularExpression rx_iss_msvc(
                    QStringLiteral(R"((Munin_msvc_v)[\d\.]+(\.exe))"));
                const QRegularExpressionMatch match_iss_msvc = rx_iss_msvc.match(line);
                if (match_iss_msvc.hasMatch()) {
                    line = line.left(match_iss_msvc.capturedStart(0)) + match_iss_msvc.captured(1)
                           + context.pt_version + match_iss_msvc.captured(2)
                           + line.mid(match_iss_msvc.capturedEnd(0));
                }

                const QRegularExpression rx_scope_ver(
                    QStringLiteral(R"((at(?:é|e) a vers(?:ã|a)o oficial\s+\*\*`?)[\d\.]+(`?\*\*))"));
                const QRegularExpressionMatch match_scope_ver = rx_scope_ver.match(line);
                if (match_scope_ver.hasMatch()) {
                    line = line.left(match_scope_ver.capturedStart(0)) + match_scope_ver.captured(1)
                           + context.pt_version + match_scope_ver.captured(2)
                           + line.mid(match_scope_ver.capturedEnd(0));
                }

                const QRegularExpression rx_heading_concl(
                    QStringLiteral(R"((## 8\. Conclus(?:ã|a)o.*?(?:da\s+vers(?:ã|a)o|vers(?:ã|a)o|-|\()\s*)[\d\.]+([^\d\.]*))"),
                    QRegularExpression::CaseInsensitiveOption);
                const QRegularExpressionMatch match_heading_concl = rx_heading_concl.match(line);
                if (match_heading_concl.hasMatch()) {
                    line = match_heading_concl.captured(1) + context.pt_version + match_heading_concl.captured(2);
                }

                const QRegularExpression rx_heading_concl_en(
                    QStringLiteral(R"((## 8\. Conclusion.*?(?:of\s+version|version|-|\()\s*)[\d\.]+([^\d\.]*))"),
                    QRegularExpression::CaseInsensitiveOption);
                const QRegularExpressionMatch match_heading_concl_en = rx_heading_concl_en.match(line);
                if (match_heading_concl_en.hasMatch()) {
                    line = match_heading_concl_en.captured(1) + context.en_version + match_heading_concl_en.captured(2);
                }

                const QRegularExpression rx_concl_ver(
                    QStringLiteral(R"(((?:plenamente operacional\s+)?na vers(?:ã|a)o\s+\*\*`?)[\d\.]+(`?\*\*))"));
                const QRegularExpressionMatch match_concl_ver = rx_concl_ver.match(line);
                if (match_concl_ver.hasMatch()) {
                    line = line.left(match_concl_ver.capturedStart(0)) + match_concl_ver.captured(1)
                           + context.pt_version + match_concl_ver.captured(2)
                           + line.mid(match_concl_ver.capturedEnd(0));
                }

                const QRegularExpression rx_concl_ver_en(
                    QStringLiteral(R"(((?:fully operational\s+)?in version\s+\*\*`?)[\d\.]+(`?\*\*))"),
                    QRegularExpression::CaseInsensitiveOption);
                const QRegularExpressionMatch match_concl_ver_en = rx_concl_ver_en.match(line);
                if (match_concl_ver_en.hasMatch()) {
                    line = line.left(match_concl_ver_en.capturedStart(0)) + match_concl_ver_en.captured(1)
                           + context.en_version + match_concl_ver_en.captured(2)
                           + line.mid(match_concl_ver_en.capturedEnd(0));
                }
            }
        } else if (key == QStringLiteral("about_pt")) {
            if (line.startsWith(QStringLiteral("Versão:")) || line.startsWith(QStringLiteral("Versao:"))) {
                line = QStringLiteral("Versão: %1").arg(context.pt_version);
            }
        } else if (key == QStringLiteral("about_en")) {
            if (line.startsWith(QStringLiteral("Version:"))) {
                line = QStringLiteral("Version: %1").arg(context.en_version);
            }
        } else if (key == QStringLiteral("clc_pt")) {
            const QRegularExpression rx_ver_date(QStringLiteral(R"(^Vers(?:ã|a)o\s+[\d\.]+,?\s*.*$)"));
            const QRegularExpression rx_close_date(QStringLiteral(R"(^(Brasil,\s+Paran(?:á|a),\s+S(?:ã|a)o Jos(?:é|e) dos Pinhais,\s+).*\.$)"));
            if (line.startsWith(QStringLiteral("Versão ")) || line.startsWith(QStringLiteral("Versao "))
                || rx_ver_date.match(line).hasMatch()) {
                line = QStringLiteral("Versão %1, %2").arg(context.pt_version, context.pt_date);
            } else {
                const QRegularExpressionMatch match = rx_close_date.match(line);
                if (match.hasMatch()) {
                    line = QStringLiteral("%1%2.").arg(match.captured(1), context.pt_date);
                }
            }
        } else if (key == QStringLiteral("clc_en")) {
            const QRegularExpression rx_ver_date(QStringLiteral(R"(^Version\s+[\d\.]+,?\s*.*$)"));
            const QRegularExpression rx_close_date(QStringLiteral(R"(^(Brazil,\s+Paran(?:á|a),\s+S(?:ã|a)o Jos(?:é|e) dos Pinhais,\s+).*\.$)"));
            if (line.startsWith(QStringLiteral("Version "))
                || rx_ver_date.match(line).hasMatch()) {
                line = QStringLiteral("Version %1, %2").arg(context.en_version, context.en_date);
            } else {
                const QRegularExpressionMatch match = rx_close_date.match(line);
                if (match.hasMatch()) {
                    line = QStringLiteral("%1%2.").arg(match.captured(1), context.en_date);
                }
            }
        } else if (key == QStringLiteral("eula_pt") || key == QStringLiteral("notice_pt")
                   || key == QStringLiteral("copyright_pt")) {
            const QRegularExpression rx_ver_date(QStringLiteral(R"(^Vers(?:ã|a)o\s+[\d\.]+,?\s*.*$)"));
            if (line.startsWith(QStringLiteral("Versão:")) || line.startsWith(QStringLiteral("Versao:"))) {
                line = QStringLiteral("Versão: %1").arg(context.pt_version);
            } else if (line.startsWith(QStringLiteral("Última atualização:"))
                       || line.startsWith(QStringLiteral("Ultima atualizacao:"))) {
                line = QStringLiteral("Última atualização: %1").arg(context.pt_date);
            } else if (line.startsWith(QStringLiteral("Data:"))) {
                line = QStringLiteral("Data: %1").arg(context.pt_date);
            } else if (line.startsWith(QStringLiteral("Versão ")) || line.startsWith(QStringLiteral("Versao "))
                       || rx_ver_date.match(line).hasMatch()) {
                line = QStringLiteral("Versão %1, %2").arg(context.pt_version, context.pt_date);
            }
        } else if (key == QStringLiteral("eula_en") || key == QStringLiteral("notice_en")
                   || key == QStringLiteral("copyright_en")) {
            const QRegularExpression rx_ver_date(QStringLiteral(R"(^Version\s+[\d\.]+,?\s*.*$)"));
            if (line.startsWith(QStringLiteral("Version:"))) {
                line = QStringLiteral("Version: %1").arg(context.en_version);
            } else if (line.startsWith(QStringLiteral("Last updated:"))) {
                line = QStringLiteral("Last updated: %1").arg(context.en_date);
            } else if (line.startsWith(QStringLiteral("Date:"))) {
                line = QStringLiteral("Date: %1").arg(context.en_date);
            } else if (line.startsWith(QStringLiteral("Version "))
                       || rx_ver_date.match(line).hasMatch()) {
                line = QStringLiteral("Version %1, %2").arg(context.en_version, context.en_date);
            }
        } else if (key == QStringLiteral("privacy_pt")) {
            if (line.startsWith(QStringLiteral("Última atualização:"))
                || line.startsWith(QStringLiteral("Ultima atualizacao:"))) {
                line = QStringLiteral("Última atualização: %1").arg(context.pt_date);
            } else if (line.startsWith(QStringLiteral("Versão:")) || line.startsWith(QStringLiteral("Versao:"))) {
                line = QStringLiteral("Versão: %1").arg(context.pt_version);
            } else {
                const QRegularExpression rx_ver_date(QStringLiteral(R"(^Vers(?:ã|a)o\s+[\d\.]+,?\s*.*$)"));
                if (rx_ver_date.match(line).hasMatch()) {
                    line = QStringLiteral("Versão %1, %2").arg(context.pt_version, context.pt_date);
                }
            }
        } else if (key == QStringLiteral("privacy_en")) {
            if (line.startsWith(QStringLiteral("Last updated:"))) {
                line = QStringLiteral("Last updated: %1").arg(context.en_date);
            } else if (line.startsWith(QStringLiteral("Version:"))) {
                line = QStringLiteral("Version: %1").arg(context.en_version);
            } else {
                const QRegularExpression rx_ver_date(QStringLiteral(R"(^Version\s+[\d\.]+,?\s*.*$)"));
                if (rx_ver_date.match(line).hasMatch()) {
                    line = QStringLiteral("Version %1, %2").arg(context.en_version, context.en_date);
                }
            }
        } else if (key == QStringLiteral("about_dialog") || key == QStringLiteral("dialog_manager")) {
            const QRegularExpression rx_pt(QStringLiteral(R"((<b>Vers(?:ã|a)o:</b>\s*)[\d\.]+)"));
            const QRegularExpression rx_en(QStringLiteral(R"((<b>Version:</b>\s*)[\d\.]+)"));

            QRegularExpressionMatch match_pt = rx_pt.match(line);
            while (match_pt.hasMatch()) {
                const QString replacement = match_pt.captured(1) + context.pt_version;
                line.replace(match_pt.capturedStart(0), match_pt.capturedLength(0), replacement);
                match_pt = rx_pt.match(line, match_pt.capturedStart(0) + replacement.length());
            }

            QRegularExpressionMatch match_en = rx_en.match(line);
            while (match_en.hasMatch()) {
                const QString replacement = match_en.captured(1) + context.en_version;
                line.replace(match_en.capturedStart(0), match_en.capturedLength(0), replacement);
                match_en = rx_en.match(line, match_en.capturedStart(0) + replacement.length());
            }
        } else if (key == QStringLiteral("main_root")) {
            const QRegularExpression rx_app_ver(QStringLiteral(R"((app\.setApplicationVersion\(QStringLiteral\(")[^"]+("\)\);))"));
            const QRegularExpressionMatch match_app_ver = rx_app_ver.match(line);
            if (match_app_ver.hasMatch()) {
                line = match_app_ver.captured(1) + context.pt_version + match_app_ver.captured(2);
            }
        } else if (key == QStringLiteral("main_source")) {
            const QRegularExpression rx_app_ver(QStringLiteral(R"((app\.setApplicationVersion\(QStringLiteral\(")[^"]+("\)\);))"));
            const QRegularExpressionMatch match_app_ver = rx_app_ver.match(line);
            if (match_app_ver.hasMatch()) {
                line = match_app_ver.captured(1) + context.pt_version + match_app_ver.captured(2);
            }

            const QRegularExpression rx_log_ver(QStringLiteral(R"((Iniciando Munin App v)[\d\.]+( ===))"));
            const QRegularExpressionMatch match_log_ver = rx_log_ver.match(line);
            if (match_log_ver.hasMatch()) {
                line = line.left(match_log_ver.capturedStart(0)) + match_log_ver.captured(1)
                       + context.pt_version + match_log_ver.captured(2)
                       + line.mid(match_log_ver.capturedEnd(0));
            }
        } else if (key == QStringLiteral("main_cli")) {
            const QRegularExpression rx_cli_ver(
                QStringLiteral(R"((static constexpr const char\*\s+APP_VERSION\s*=\s*")[^"]+("\s*;))"));
            const QRegularExpressionMatch match_cli_ver = rx_cli_ver.match(line);
            if (match_cli_ver.hasMatch()) {
                line = match_cli_ver.captured(1) + context.pt_version + match_cli_ver.captured(2);
            }
        } else if (key == QStringLiteral("release_doc")) {
            const QRegularExpression rx_rel_title(
                QStringLiteral(R"((# Notas de Lançamento / Release Notes — v)[\d\.]+)"));
            const QRegularExpressionMatch match_rel_title = rx_rel_title.match(line);
            if (match_rel_title.hasMatch()) {
                line = match_rel_title.captured(1) + context.pt_version;
            }

            const QRegularExpression rx_rel_h1(
                QStringLiteral(R"((# Munin App — Release v)[\d\.]+)"));
            const QRegularExpressionMatch match_rel_h1 = rx_rel_h1.match(line);
            if (match_rel_h1.hasMatch()) {
                line = match_rel_h1.captured(1) + context.pt_version;
            }

            const QRegularExpression rx_rel_bold(
                QStringLiteral(R"((\*\*Munin App v)[\d\.]+(\*\*))"));
            const QRegularExpressionMatch match_rel_bold = rx_rel_bold.match(line);
            if (match_rel_bold.hasMatch()) {
                line = match_rel_bold.captured(1) + context.pt_version + match_rel_bold.captured(2);
            }

            const QRegularExpression rx_tag_pt(
                QStringLiteral(R"((\*\*Tag de Vers(?:ã|a)o:\*\*\s*`v?)[^`]+(`))"));
            const QRegularExpressionMatch match_tag_pt = rx_tag_pt.match(line);
            if (match_tag_pt.hasMatch()) {
                line = match_tag_pt.captured(1) + context.pt_version + match_tag_pt.captured(2)
                       + (line.endsWith(QStringLiteral("  ")) ? QStringLiteral("  ") : QString());
            }

            const QRegularExpression rx_pub_pt(
                QStringLiteral(R"((\*\*Data de Publica(?:çã|ca)o:\*\*\s*).*$)"));
            const QRegularExpressionMatch match_pub_pt = rx_pub_pt.match(line);
            if (match_pub_pt.hasMatch()) {
                line = match_pub_pt.captured(1) + context.pt_date
                       + (line.endsWith(QStringLiteral("  ")) ? QStringLiteral("  ") : QString());
            }

            const QRegularExpression rx_tag_en(
                QStringLiteral(R"((\*\*Release Tag:\*\*\s*`v?)[^`]+(`))"));
            const QRegularExpressionMatch match_tag_en = rx_tag_en.match(line);
            if (match_tag_en.hasMatch()) {
                line = match_tag_en.captured(1) + context.en_version + match_tag_en.captured(2)
                       + (line.endsWith(QStringLiteral("  ")) ? QStringLiteral("  ") : QString());
            }

            const QRegularExpression rx_pub_en(
                QStringLiteral(R"((\*\*Publication Date:\*\*\s*).*$)"));
            const QRegularExpressionMatch match_pub_en = rx_pub_en.match(line);
            if (match_pub_en.hasMatch()) {
                line = match_pub_en.captured(1) + context.en_date
                       + (line.endsWith(QStringLiteral("  ")) ? QStringLiteral("  ") : QString());
            }

            if (line.contains(QStringLiteral("Munin-v"))) {
                const QRegularExpression rx_art(
                    QStringLiteral(R"((Munin-v)[\d\.]+(-x64-(?:Setup\.exe|Portable\.zip)))"));
                line.replace(rx_art, QStringLiteral("\\1%1\\2").arg(context.pt_version));
            }
        } else if (key == QStringLiteral("installer_unified") || key == QStringLiteral("installer_mingw")
                   || key == QStringLiteral("installer_msvc")) {
            if (line.startsWith(QStringLiteral("#define MyAppVersion"))) {
                const QRegularExpression rx(QStringLiteral(R"((#define\s+MyAppVersion\s+")[\d\.]+(";?))"));
                const QRegularExpressionMatch match = rx.match(line);
                if (match.hasMatch()) {
                    line = match.captured(1) + context.pt_version + match.captured(2);
                }
            } else if (key == QStringLiteral("installer_mingw")
                       && line.startsWith(QStringLiteral("OutputBaseFilename=Munin_mingw_v"))) {
                line = QStringLiteral("OutputBaseFilename=Munin_mingw_v%1").arg(context.pt_version);
            } else if (key == QStringLiteral("installer_msvc")
                       && line.trimmed().startsWith(QStringLiteral("OutputBaseFilename=Munin_msvc_v"))) {
                const QString indentation = line.left(line.size() - line.trimmed().size());
                line = QStringLiteral("%1OutputBaseFilename=Munin_msvc_v%2")
                           .arg(indentation, context.pt_version);
            }
        }

        if (line != original) {
            changed = true;
        }
    }

    return changed;
}

QList<UpdateResult> apply_updates(const UpdateContext &context, const QString &base_dir)
{
    QList<UpdateResult> results;

    const auto specs = file_specs(base_dir);
    for (const FileSpec &spec : specs) {
        UpdateResult result;
        result.key = spec.key;
        result.group = spec.group.isEmpty() ? group_source() : spec.group;
        result.relative_path = spec.relative_path;

        const QString path = resolve_file_path(base_dir, spec.relative_path);
        const std::optional<QString> text = read_file_text(path);
        if (!text.has_value()) {
            result.ok = false;
            result.message = QStringLiteral("arquivo inexistente");
            results.append(result);
            continue;
        }

        const QString line_ending = detect_line_ending(text.value());
        QStringList lines = split_lines_preserving_empty(text.value());
        const bool changed = apply_line_updates(spec_content_key(spec), &lines, context);

        if (!changed) {
            bool all_patterns_present = true;
            const QStringList pattern_list = patterns().value(spec_content_key(spec));
            for (const QString &pattern : pattern_list) {
                const QRegularExpression rx(pattern, QRegularExpression::MultilineOption);
                if (!rx.match(text.value()).hasMatch()) {
                    all_patterns_present = false;
                    break;
                }
            }

            if (all_patterns_present) {
                result.ok = true;
                result.message = QStringLiteral("já atualizado");
            } else {
                result.ok = false;
                result.message = QStringLiteral("padrões não encontrados / sem alteração");
            }
            results.append(result);
            continue;
        }

        QString target_path = path;
        QString target_rel_path = spec.relative_path;

        // Se for release_doc e a versão mudou, renomeia docs/release-v<old>.md para docs/release-v<new>.md
        if (spec.content_key == QStringLiteral("release_doc")) {
            const QString new_rel_path = QStringLiteral("docs/release-v%1.md").arg(context.pt_version);
            const QString new_path = resolve_file_path(base_dir, new_rel_path);
            target_path = new_path;
            target_rel_path = new_rel_path;
        }

        QString error_message;
        if (write_file_text(target_path, join_lines(lines, line_ending), &error_message)) {
            if (target_path != path) {
                QFile::remove(path);
            }
            result.relative_path = target_rel_path;
            result.ok = true;
            result.message = QStringLiteral("atualizado");
        } else {
            result.ok = false;
            result.message = QStringLiteral("erro escrita: %1").arg(error_message);
        }

        results.append(result);
    }

    return results;
}

VersionEditorWidget::VersionEditorWidget(QWidget *parent) : QWidget(parent), selected_base_dir_(detect_project_root())
{
    setWindowTitle(QStringLiteral("Editor de Versões / Datas - Munin App (PT/EN)"));
    resize(980, 640);

    const QDate today = QDate::currentDate();
    const QString default_version = build_version_from_date(today);
    const int default_day = today.isValid() ? today.day() : 1;
    const int default_month_index = today.isValid() ? qMax(0, today.month() - 1) : 0;
    const int default_year = today.isValid() ? today.year() : 2026;

    const QString icon_path = find_icon_path(selected_base_dir_, QStringLiteral("munin.ico"));
    if (!icon_path.isEmpty()) {
        const QIcon icon(icon_path);
        QApplication::setWindowIcon(icon);
        setWindowIcon(icon);
    }

    auto *layout = new QVBoxLayout(this);

    auto *folder_layout = new QHBoxLayout();
    folder_layout->addWidget(new QLabel(QStringLiteral("Pasta dos arquivos:"), this));

    folder_path_ = new QLineEdit(selected_base_dir_, this);
    folder_path_->setReadOnly(true);
    folder_layout->addWidget(folder_path_);

    select_folder_btn_ = new QPushButton(QStringLiteral("Selecionar..."), this);
    folder_layout->addWidget(select_folder_btn_);
    layout->addLayout(folder_layout);

    auto *pt_group = new QGroupBox(QStringLiteral("Português (pt-BR)"), this);
    auto *pt_layout = new QHBoxLayout(pt_group);
    pt_layout->addWidget(new QLabel(QStringLiteral("Versão:"), pt_group));
    pt_version_ = new QLineEdit(default_version, pt_group);
    pt_layout->addWidget(pt_version_);

    pt_layout->addWidget(new QLabel(QStringLiteral("Dia:"), pt_group));
    pt_day_ = new QSpinBox(pt_group);
    pt_day_->setRange(1, 31);
    pt_day_->setValue(default_day);
    pt_layout->addWidget(pt_day_);

    pt_layout->addWidget(new QLabel(QStringLiteral("Mês:"), pt_group));
    pt_month_ = new QComboBox(pt_group);
    pt_month_->addItems(kPtMonths);
    pt_month_->setCurrentIndex(default_month_index);
    pt_layout->addWidget(pt_month_);

    pt_layout->addWidget(new QLabel(QStringLiteral("Ano:"), pt_group));
    pt_year_ = new QSpinBox(pt_group);
    pt_year_->setRange(1900, 3000);
    pt_year_->setValue(default_year);
    pt_layout->addWidget(pt_year_);
    layout->addWidget(pt_group);

    auto *en_group = new QGroupBox(QStringLiteral("English (en-US)"), this);
    auto *en_layout = new QHBoxLayout(en_group);
    en_layout->addWidget(new QLabel(QStringLiteral("Version:"), en_group));
    en_version_ = new QLineEdit(default_version, en_group);
    en_layout->addWidget(en_version_);

    en_layout->addWidget(new QLabel(QStringLiteral("Day:"), en_group));
    en_day_ = new QSpinBox(en_group);
    en_day_->setRange(1, 31);
    en_day_->setValue(default_day);
    en_layout->addWidget(en_day_);

    en_layout->addWidget(new QLabel(QStringLiteral("Month:"), en_group));
    en_month_ = new QComboBox(en_group);
    en_month_->addItems(kEnMonths);
    en_month_->setCurrentIndex(default_month_index);
    en_layout->addWidget(en_month_);

    en_layout->addWidget(new QLabel(QStringLiteral("Year:"), en_group));
    en_year_ = new QSpinBox(en_group);
    en_year_->setRange(1900, 3000);
    en_year_->setValue(default_year);
    en_layout->addWidget(en_year_);
    layout->addWidget(en_group);

    auto *btn_layout = new QHBoxLayout();
    check_btn_ = new QPushButton(QStringLiteral("Verificar arquivos"), this);
    save_btn_ = new QPushButton(QStringLiteral("Salvar alterações"), this);
    refresh_btn_ = new QPushButton(QStringLiteral("Recarregar verificação"), this);

    const QString check_icon = find_icon_path(selected_base_dir_, QStringLiteral("munin.png"));
    if (!check_icon.isEmpty()) {
        check_btn_->setIcon(QIcon(check_icon));
    }
    const QString save_icon = find_icon_path(selected_base_dir_, QStringLiteral("munin_300-300.png"));
    if (!save_icon.isEmpty()) {
        save_btn_->setIcon(QIcon(save_icon));
    }
    const QString calendar_icon = find_icon_path(selected_base_dir_, QStringLiteral("munin_150-150.png"));
    if (!calendar_icon.isEmpty()) {
        refresh_btn_->setIcon(QIcon(calendar_icon));
        select_folder_btn_->setIcon(QIcon(calendar_icon));
    }

    btn_layout->addWidget(check_btn_);
    btn_layout->addWidget(save_btn_);
    btn_layout->addWidget(refresh_btn_);
    layout->addLayout(btn_layout);

    status_tabs_ = new QTabWidget(this);
    status_tabs_->setDocumentMode(true);
    status_tabs_->setUsesScrollButtons(true);
    status_tabs_->setElideMode(Qt::ElideRight);
    layout->addWidget(status_tabs_, 1);

    connect(select_folder_btn_, &QPushButton::clicked, this, [this]() { selectBaseDirectory(); });
    connect(check_btn_, &QPushButton::clicked, this, [this]() { checkFiles(); });
    connect(save_btn_, &QPushButton::clicked, this, [this]() { saveChanges(); });
    connect(refresh_btn_, &QPushButton::clicked, this, [this]() { checkFiles(); });

    checkFiles();
}

void VersionEditorWidget::selectBaseDirectory()
{
    const QString selected = QFileDialog::getExistingDirectory(
        this, QStringLiteral("Selecione a pasta onde os arquivos estão"), selected_base_dir_);
    if (selected.isEmpty()) {
        return;
    }

    selected_base_dir_ = normalize_dir(selected);
    folder_path_->setText(selected_base_dir_);
    checkFiles();
}

void VersionEditorWidget::checkFiles()
{
    const QList<FileStatus> statuses = check_expected_lines(selected_base_dir_);
    QMap<QString, QStringList> grouped_lines;
    for (const QString &group : group_order()) {
        grouped_lines[group].append(QStringLiteral("Base: %1").arg(selected_base_dir_));
        grouped_lines[group].append(QString());
    }

    for (const FileStatus &status : statuses) {
        QStringList &lines = grouped_lines[status.group];
        lines.append(QStringLiteral("%1").arg(status.relative_path));
        lines.append(QStringLiteral("  chave: %1").arg(status.key));
        lines.append(QStringLiteral("  arquivo: %1").arg(status.exists ? QStringLiteral("existe") : QStringLiteral("ausente")));

        if (status.exists) {
            if (!status.found.isEmpty()) {
                lines.append(QStringLiteral("  encontrado:"));
                for (const QString &entry : status.found) {
                    lines.append(QStringLiteral("    - %1").arg(entry));
                }
            }

            if (!status.missing.isEmpty()) {
                lines.append(QStringLiteral("  ausente no conteúdo:"));
                for (const QString &entry : status.missing) {
                    lines.append(QStringLiteral("    - %1").arg(entry));
                }
            }
        }

        lines.append(QString());
    }

    QList<QPair<QString, QStringList>> groups;
    for (const QString &group : group_order()) {
        groups.append({group, grouped_lines.value(group)});
    }
    setStatusGroups(groups);
}

void VersionEditorWidget::saveChanges()
{
    const QString pt_version = pt_version_->text().trimmed();
    const int pt_day = pt_day_->value();
    const int pt_month_index = pt_month_->currentIndex();
    const int pt_year = pt_year_->value();

    const QString en_version = en_version_->text().trimmed();
    const int en_day = en_day_->value();
    const int en_month_index = en_month_->currentIndex();
    const int en_year = en_year_->value();

    const UpdateContext context = {
        pt_version,
        QStringLiteral("%1 de %2 de %3")
            .arg(QString::number(pt_day), kPtMonthsLower.value(pt_month_index), QString::number(pt_year)),
        en_version,
        QStringLiteral("%1 %2, %3")
            .arg(kEnMonths.value(en_month_index), QString::number(en_day), QString::number(en_year)),
        readme_version_token(pt_version),
        readme_version_token(en_version),
    };

    const QList<UpdateResult> results = apply_updates(context, selected_base_dir_);

    QMap<QString, QStringList> grouped_lines;
    for (const QString &group : group_order()) {
        grouped_lines[group].append(QStringLiteral("Resultados:"));
        grouped_lines[group].append(QString());
    }

    for (const UpdateResult &result : results) {
        QStringList &lines = grouped_lines[result.group];
        lines.append(QStringLiteral("%1: %2 - %3")
                         .arg(result.relative_path,
                              result.ok ? QStringLiteral("OK") : QStringLiteral("FAIL"),
                              result.message));
    }

    QList<QPair<QString, QStringList>> groups;
    for (const QString &group : group_order()) {
        groups.append({group, grouped_lines.value(group)});
    }
    setStatusGroups(groups);
}

void VersionEditorWidget::setStatusLines(const QStringList &lines)
{
    QList<QPair<QString, QStringList>> groups;
    groups.append(qMakePair(group_about(), lines));
    setStatusGroups(groups);
}

void VersionEditorWidget::setStatusGroups(const QList<QPair<QString, QStringList>> &groups)
{
    if (status_tabs_ == nullptr) {
        return;
    }

    const QString current_title =
        status_tabs_->currentIndex() >= 0 ? status_tabs_->tabText(status_tabs_->currentIndex()) : QString();

    while (status_tabs_->count() > 0) {
        QWidget *page = status_tabs_->widget(0);
        status_tabs_->removeTab(0);
        if (page != nullptr) {
            page->deleteLater();
        }
    }

    int target_index = 0;
    for (const auto &group : groups) {
        auto *editor = new QTextEdit(status_tabs_);
        editor->setReadOnly(true);
        editor->setLineWrapMode(QTextEdit::NoWrap);
        QStringList lines = group.second;
        if (lines.isEmpty()) {
            lines.append(QStringLiteral("Nenhum arquivo neste grupo."));
        }
        editor->setPlainText(lines.join(QLatin1Char('\n')));

        const int index = status_tabs_->addTab(editor, group.first);
        if (!current_title.isEmpty() && group.first == current_title) {
            target_index = index;
        }
    }

    if (status_tabs_->count() > 0) {
        status_tabs_->setCurrentIndex(qBound(0, target_index, status_tabs_->count() - 1));
    }
}

}  // namespace mocks::version_editor

int main(int argc, char *argv[])
{
    if (argc > 1) {
        const QString arg = QString::fromLocal8Bit(argv[1]).trimmed().toLower();
        if (arg == QStringLiteral("--check") || arg == QStringLiteral("-c") || arg == QStringLiteral("--status")) {
#if defined(Q_OS_WIN)
            if (AttachConsole(ATTACH_PARENT_PROCESS)) {
                HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
                if (hOut == nullptr || hOut == INVALID_HANDLE_VALUE || GetFileType(hOut) == FILE_TYPE_UNKNOWN) {
                    FILE *fp = nullptr;
                    freopen_s(&fp, "CONOUT$", "w", stdout);
                    freopen_s(&fp, "CONOUT$", "w", stderr);
                }
            }
#endif
            QTextStream out(stdout);
            const QString base_dir = (argc > 2) ? mocks::version_editor::normalize_dir(QString::fromLocal8Bit(argv[2]))
                                                : mocks::version_editor::detect_project_root();
            const auto statuses = mocks::version_editor::check_expected_lines(base_dir);
            bool all_ok = true;
            for (const auto &status : statuses) {
                if (!status.exists) {
                    all_ok = false;
                    out << "[AUSENTE] " << status.relative_path << "\n";
                } else if (!status.missing.isEmpty()) {
                    all_ok = false;
                    out << "[PADRAO AUSENTE] " << status.relative_path << " : " << status.missing.join(QStringLiteral(" | ")) << "\n";
                } else {
                    out << "[OK] " << status.relative_path << "\n";
                }
            }
            out << (all_ok ? "TODOS OS ARQUIVOS VERIFICADOS COM SUCESSO.\n"
                           : "FALHAS NA VERIFICACAO DE ARQUIVOS.\n");
            out.flush();
            return all_ok ? 0 : 1;
        } else if (arg == QStringLiteral("--apply") || arg == QStringLiteral("--save") || arg == QStringLiteral("--update")) {
#if defined(Q_OS_WIN)
            if (AttachConsole(ATTACH_PARENT_PROCESS)) {
                HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
                if (hOut == nullptr || hOut == INVALID_HANDLE_VALUE || GetFileType(hOut) == FILE_TYPE_UNKNOWN) {
                    FILE *fp = nullptr;
                    freopen_s(&fp, "CONOUT$", "w", stdout);
                    freopen_s(&fp, "CONOUT$", "w", stderr);
                }
            }
#endif
            QTextStream out(stdout);
            const QString base_dir = (argc > 2) ? mocks::version_editor::normalize_dir(QString::fromLocal8Bit(argv[2]))
                                                : mocks::version_editor::detect_project_root();
            const QDate today = QDate::currentDate();
            const QString version = (argc > 3) ? QString::fromLocal8Bit(argv[3]).trimmed()
                                               : mocks::version_editor::build_version_from_date(today);
            const QDate target_date = (argc > 3)
                                          ? mocks::version_editor::parse_date_from_version(version)
                                          : today;
            const int day = target_date.day();
            const int month_idx = qMax(0, target_date.month() - 1);
            const int year = target_date.year();

            const mocks::version_editor::UpdateContext ctx = {
                version,
                QStringLiteral("%1 de %2 de %3").arg(QString::number(day), mocks::version_editor::kPtMonthsLower.value(month_idx), QString::number(year)),
                version,
                QStringLiteral("%1 %2, %3").arg(mocks::version_editor::kEnMonths.value(month_idx), QString::number(day), QString::number(year)),
                mocks::version_editor::readme_version_token(version),
                mocks::version_editor::readme_version_token(version),
            };

            const auto results = mocks::version_editor::apply_updates(ctx, base_dir);
            bool all_ok = true;
            for (const auto &res : results) {
                if (!res.ok) {
                    all_ok = false;
                    out << "[FALHA] " << res.relative_path << " - " << res.message << "\n";
                } else if (res.message == QStringLiteral("já atualizado")) {
                    out << "[OK] " << res.relative_path << " (já atualizado)\n";
                } else {
                    out << "[ATUALIZADO] " << res.relative_path << "\n";
                }
            }
            out << (all_ok ? "TODOS OS ARQUIVOS ATUALIZADOS COM SUCESSO.\n"
                           : "FALHAS NA ATUALIZACAO DE ARQUIVOS.\n");
            out.flush();
            return all_ok ? 0 : 1;
        }
    }

    QApplication app(argc, argv);
    mocks::version_editor::VersionEditorWidget widget;
    widget.show();
    return app.exec();
}
