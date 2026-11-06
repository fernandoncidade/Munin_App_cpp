#include "LogManager.hpp"
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <iostream>

LogManager& LogManager::instance() {
    static LogManager s_instance;
    return s_instance;
}

LogManager::LogManager() {
    init();
}

LogManager::~LogManager() {
    QMutexLocker locker(&m_mutex);
    if (m_logFile.isOpen()) {
        m_textStream.flush();
        m_logFile.close();
    }
}

void LogManager::init() {
    QMutexLocker locker(&m_mutex);
    if (m_configured) {
        return;
    }

    QString localAppData = qEnvironmentVariable("LOCALAPPDATA");
    if (localAppData.isEmpty()) {
        localAppData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    }

    QDir logDir(QDir(localAppData).filePath(QStringLiteral("Munin App/logs")));
    if (!logDir.exists()) {
        logDir.mkpath(QStringLiteral("."));
    }

    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss"));
    m_logFilePath = logDir.filePath(QStringLiteral("munin_%1.log").arg(timestamp));

    m_logFile.setFileName(m_logFilePath);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_textStream.setDevice(&m_logFile);
        m_configured = true;
    } else {
        std::cerr << "Falha ao abrir arquivo de log: " << m_logFilePath.toStdString() << std::endl;
    }

    qInstallMessageHandler(qtMessageHandler);
}

void LogManager::configure_logging() {
    instance().init();
}

void LogManager::writeMessage(const QString& level, const QString& message) {
    QMutexLocker locker(&m_mutex);
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"));
    QString formatted = QStringLiteral("%1 [%2] Munin App: %3").arg(timestamp, level, message);

    std::cout << formatted.toStdString() << std::endl;

    if (m_configured && m_logFile.isOpen()) {
        m_textStream << formatted << "\n";
        m_textStream.flush();
    }
}

void LogManager::debug(const QString& message) {
    instance().writeMessage(QStringLiteral("DEBUG"), message);
}

void LogManager::info(const QString& message) {
    instance().writeMessage(QStringLiteral("INFO"), message);
}

void LogManager::warning(const QString& message) {
    instance().writeMessage(QStringLiteral("WARNING"), message);
}

void LogManager::error(const QString& message, bool exc_info) {
    Q_UNUSED(exc_info);
    instance().writeMessage(QStringLiteral("ERROR"), message);
}

void LogManager::critical(const QString& message, bool exc_info) {
    Q_UNUSED(exc_info);
    instance().writeMessage(QStringLiteral("CRITICAL"), message);
}

QString LogManager::get_log_file() {
    return instance().m_logFilePath;
}

QString LogManager::getLogFile() {
    return instance().m_logFilePath;
}

void LogManager::qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    Q_UNUSED(context);
    switch (type) {
    case QtDebugMsg:
        LogManager::debug(msg);
        break;
    case QtInfoMsg:
        LogManager::info(msg);
        break;
    case QtWarningMsg:
        LogManager::warning(msg);
        break;
    case QtCriticalMsg:
        LogManager::error(msg);
        break;
    case QtFatalMsg:
        LogManager::critical(msg);
        break;
    }
}
