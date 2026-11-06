#pragma once

#include <QString>
#include <QIcon>

QString get_app_base_path();
QString getAppBasePath();

QString get_text_file_path(const QString& filename, const QString& folder = QString());
QString getTextFilePath(const QString& filename, const QString& folder = QString());

QString load_text_file(const QString& filename, const QString& folder = QString(), const QString& encoding = QStringLiteral("utf-8"));
QString loadTextFile(const QString& filename, const QString& folder = QString(), const QString& encoding = QStringLiteral("utf-8"));

QString get_app_icon_path();
QIcon get_app_icon();

