#include "ApplicationPathUtils.hpp"
#include "LogManager.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>

QString get_app_base_path() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString currentDir = QDir::currentPath();

    // Candidate root folders
    QStringList candidates = {
        appDir,
        QDir(appDir).filePath(QStringLiteral("..")),
        QDir(appDir).filePath(QStringLiteral("../..")),
        QDir(appDir).filePath(QStringLiteral("../../source")),
        QDir(appDir).filePath(QStringLiteral("../source")),
        QDir(appDir).filePath(QStringLiteral("source")),
        currentDir,
        QDir(currentDir).filePath(QStringLiteral("source")),
        QDir(currentDir).filePath(QStringLiteral("..")),
        QDir(currentDir).filePath(QStringLiteral("../..")),
        QDir(currentDir).filePath(QStringLiteral("../source")),
        QStringLiteral("C:\\Program Files\\Munin App"),
        QStringLiteral("C:\\Program Files (x86)\\Munin App"),
        QStringLiteral("C:\\Program Files\\Munin"),
        QStringLiteral("C:\\Program Files (x86)\\Munin")
    };

    for (const QString& candidate : candidates) {
        QDir dir(candidate);
        if (dir.exists(QStringLiteral("assets")) || dir.exists(QStringLiteral("source/assets"))) {
            return dir.absolutePath();
        }
    }

    return QDir(appDir).absolutePath();
}

QString getAppBasePath() {
    return get_app_base_path();
}

QString get_text_file_path(const QString& filename, const QString& folder) {
    try {
        QString basePath = get_app_base_path();
        QString appDir = QCoreApplication::applicationDirPath();
        QString currentDir = QDir::currentPath();

        QStringList baseRoots = {
            basePath,
            QDir(basePath).filePath(QStringLiteral("source")),
            QDir(basePath).filePath(QStringLiteral("..")),
            appDir,
            QDir(appDir).filePath(QStringLiteral("..")),
            QDir(appDir).filePath(QStringLiteral("../..")),
            QDir(appDir).filePath(QStringLiteral("../../source")),
            currentDir,
            QDir(currentDir).filePath(QStringLiteral("source")),
            QStringLiteral("C:\\Program Files\\Munin App"),
            QStringLiteral("C:\\Program Files (x86)\\Munin App"),
            QStringLiteral("C:\\Program Files\\Munin"),
            QStringLiteral("C:\\Program Files (x86)\\Munin")
        };

        QStringList possiblePaths;
        for (const QString& root : baseRoots) {
            if (!folder.isEmpty()) {
                possiblePaths << QDir(root).filePath(QStringLiteral("assets/%1/%2").arg(folder, filename));
                possiblePaths << QDir(root).filePath(QStringLiteral("%1/%2").arg(folder, filename));
                possiblePaths << QDir(root).filePath(QStringLiteral("_internal/assets/%1/%2").arg(folder, filename));
                possiblePaths << QDir(root).filePath(QStringLiteral("_internal/%1/%2").arg(folder, filename));
            } else {
                possiblePaths << QDir(root).filePath(QStringLiteral("assets/%1").arg(filename));
                possiblePaths << QDir(root).filePath(filename);
                possiblePaths << QDir(root).filePath(QStringLiteral("_internal/assets/%1").arg(filename));
                possiblePaths << QDir(root).filePath(QStringLiteral("_internal/%1").arg(filename));
            }
        }

        for (const QString& p : possiblePaths) {
            QFileInfo fi(p);
            if (fi.exists() && fi.isFile()) {
                return fi.absoluteFilePath();
            }
        }

        return QFileInfo(possiblePaths.value(0)).absoluteFilePath();
    } catch (const std::exception& e) {
        LogManager::error(QStringLiteral("Erro ao obter caminho do arquivo de texto '%1': %2").arg(filename, QString::fromLatin1(e.what())));
        return QString();
    }
}

QString getTextFilePath(const QString& filename, const QString& folder) {
    return get_text_file_path(filename, folder);
}

QString load_text_file(const QString& filename, const QString& folder, const QString& encoding) {
    Q_UNUSED(encoding);
    try {
        QString filePath = get_text_file_path(filename, folder);
        if (filePath.isEmpty() || !QFile::exists(filePath)) {
            LogManager::error(QStringLiteral("Arquivo de texto '%1' não encontrado.").arg(filename));
            return QString();
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            LogManager::error(QStringLiteral("Erro ao abrir arquivo de texto '%1'.").arg(filename));
            return QString();
        }

        QByteArray bytes = file.readAll();
        file.close();

        // Handle UTF-8 BOM if present
        if (bytes.startsWith("\xEF\xBB\xBF")) {
            bytes = bytes.mid(3);
        }

        return QString::fromUtf8(bytes);
    } catch (const std::exception& e) {
        LogManager::error(QStringLiteral("Erro ao carregar arquivo de texto '%1': %2").arg(filename, QString::fromLatin1(e.what())));
        return QString();
    }
}

QString loadTextFile(const QString& filename, const QString& folder, const QString& encoding) {
    return load_text_file(filename, folder, encoding);
}

QString get_app_icon_path() {
    QString basePath = get_app_base_path();
    QString appDir = QCoreApplication::applicationDirPath();
    QString currentDir = QDir::currentPath();

    QStringList candidates = {
        QDir(basePath).filePath(QStringLiteral("assets/icons/munin.ico")),
        QDir(basePath).filePath(QStringLiteral("source/assets/icons/munin.ico")),
        QDir(appDir).filePath(QStringLiteral("assets/icons/munin.ico")),
        QDir(appDir).filePath(QStringLiteral("source/assets/icons/munin.ico")),
        QDir(appDir).filePath(QStringLiteral("../source/assets/icons/munin.ico")),
        QDir(currentDir).filePath(QStringLiteral("assets/icons/munin.ico")),
        QDir(currentDir).filePath(QStringLiteral("source/assets/icons/munin.ico")),
        QStringLiteral("C:/Users/ferna/APLICATIVOS/CPP/Munin_App_cpp/source/assets/icons/munin.ico")
    };

    for (const QString& candidate : candidates) {
        if (QFile::exists(candidate)) {
            return QDir::toNativeSeparators(candidate);
        }
    }
    return QString();
}

QIcon get_app_icon() {
    QString path = get_app_icon_path();
    if (!path.isEmpty() && QFile::exists(path)) {
        return QIcon(path);
    }
    return QIcon();
}

