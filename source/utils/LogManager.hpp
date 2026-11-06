#pragma once

#include <QString>
#include <QRecursiveMutex>
#include <QFile>
#include <QTextStream>
#include <QtLogging>

class LogManager {
public:
    static LogManager& instance();
    static LogManager* get_logger() { return &instance(); }

    void log(const QString& message, const QString& level = QStringLiteral("INFO")) {
        writeMessage(level, message);
    }

    static void debug(const QString& message);
    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message, bool exc_info = false);
    static void critical(const QString& message, bool exc_info = true);

    static QString get_log_file();
    static QString getLogFile();
    static void configure_logging();

private:
    LogManager();
    ~LogManager();

    void init();
    void writeMessage(const QString& level, const QString& message);
    static void qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

    QFile m_logFile;
    QTextStream m_textStream;
    QRecursiveMutex m_mutex;
    QString m_logFilePath;
    bool m_configured = false;
};

namespace source::utils {
    using LogManager = ::LogManager;
}
