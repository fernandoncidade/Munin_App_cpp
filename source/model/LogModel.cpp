#include "LogModel.hpp"
#include "../engine/TimeSliceScanner.hpp"

#include <QColor>
#include <QCoreApplication>

namespace Munin::Model {

LogModel::LogModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_monoFont(QStringList{QStringLiteral("Cascadia Code"), QStringLiteral("Consolas"), QStringLiteral("Segoe UI Mono"), QStringLiteral("monospace")}) {
    m_monoFont.setStyleHint(QFont::Monospace);
    m_monoFont.setPointSize(10);
}

void LogModel::setEngine(
    std::shared_ptr<Engine::MemoryMappedFile> mmap,
    std::shared_ptr<Engine::ChunkIndex> index
) {
    beginResetModel();
    m_mmap = std::move(mmap);
    m_index = std::move(index);
    m_isFiltered = false;
    m_filteredLines.clear();
    m_filterDescription.clear();
    endResetModel();
}

int LogModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid() || !m_mmap || !m_index) {
        return 0;
    }

    if (m_isFiltered) {
        return static_cast<int>(std::min<uint64_t>(m_filteredLines.size(), static_cast<size_t>(INT_MAX)));
    }

    uint64_t total = m_index->totalLines();
    return static_cast<int>(std::min<uint64_t>(total, static_cast<uint64_t>(INT_MAX)));
}

int LogModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return ColumnCount;
}

uint64_t LogModel::mapRowToRealLine(int row) const noexcept {
    if (row < 0) return 0;
    if (m_isFiltered) {
        size_t idx = static_cast<size_t>(row);
        if (idx < m_filteredLines.size()) {
            return m_filteredLines[idx];
        }
        return 0;
    }
    return static_cast<uint64_t>(row);
}

std::string_view LogModel::getRawLine(uint64_t realLine) const {
    if (!m_mmap || !m_index || !m_mmap->isOpen()) {
        return {};
    }
    return m_index->getLine(m_mmap->data(), m_mmap->size(), realLine);
}

QString LogModel::getLineString(uint64_t realLine) const {
    std::string_view raw = getRawLine(realLine);
    if (raw.empty()) return QString();
    return QString::fromUtf8(raw.data(), static_cast<qsizetype>(raw.size()));
}

LogLevel LogModel::detectLogLevel(std::string_view line) noexcept {
    if (line.empty()) return LogLevel::Unknown;

    size_t scanLen = std::min<size_t>(line.size(), 80);
    std::string_view head = line.substr(0, scanLen);

    if (head.find("FATAL") != std::string_view::npos ||
        head.find("CRITICAL") != std::string_view::npos ||
        head.find("EMERG") != std::string_view::npos) {
        return LogLevel::Fatal;
    }
    if (head.find("ERROR") != std::string_view::npos ||
        head.find("ERR") != std::string_view::npos ||
        head.find("EXCEPTION") != std::string_view::npos ||
        head.find("SEVERE") != std::string_view::npos) {
        return LogLevel::Error;
    }
    if (head.find("WARN") != std::string_view::npos ||
        head.find("WARNING") != std::string_view::npos) {
        return LogLevel::Warn;
    }
    if (head.find("INFO") != std::string_view::npos ||
        head.find("NOTICE") != std::string_view::npos) {
        return LogLevel::Info;
    }
    if (head.find("DEBUG") != std::string_view::npos) {
        return LogLevel::Debug;
    }
    if (head.find("TRACE") != std::string_view::npos) {
        return LogLevel::Trace;
    }

    return LogLevel::Unknown;
}

QString LogModel::formatLogLevel(LogLevel level) {
    switch (level) {
    case LogLevel::Fatal: return QStringLiteral("FATAL");
    case LogLevel::Error: return QStringLiteral("ERROR");
    case LogLevel::Warn:  return QStringLiteral("WARN");
    case LogLevel::Info:  return QStringLiteral("INFO");
    case LogLevel::Debug: return QStringLiteral("DEBUG");
    case LogLevel::Trace: return QStringLiteral("TRACE");
    default:              return QString();
    }
}

QVariant LogModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || !m_mmap || !m_index) {
        return {};
    }

    uint64_t realLine = mapRowToRealLine(index.row());

    if (role == RawTextRole) {
        return getLineString(realLine);
    }

    if (role == LineNumberRole) {
        return static_cast<qulonglong>(realLine + 1);
    }

    if (role == LogLevelRole) {
        std::string_view raw = getRawLine(realLine);
        return static_cast<int>(detectLogLevel(raw));
    }

    if (role == ByteOffsetRole) {
        Engine::LineSpan span = m_index->getLineSpan(m_mmap->data(), m_mmap->size(), realLine);
        return static_cast<qulonglong>(span.offset);
    }

    if (role == IsFilteredRole) {
        return m_isFiltered;
    }

    if (role == Qt::FontRole) {
        return m_monoFont;
    }

    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case ColLineNumber:
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        case ColLevel:
            return static_cast<int>(Qt::AlignCenter);
        default:
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    if (role == Qt::DisplayRole) {
        std::string_view raw = getRawLine(realLine);

        switch (index.column()) {
        case ColLineNumber:
            return QString::number(realLine + 1);

        case ColTimestamp: {
            if (raw.size() >= 19) {
                size_t start = (raw[0] == '[' || raw[0] == '{' || raw[0] == '"') ? 1 : 0;
                if (raw.size() >= start + 19) {
                    char c4 = raw[start + 4];
                    char c7 = raw[start + 7];
                    if (c4 == '-' && c7 == '-') {
                        return QString::fromUtf8(raw.data() + start, 19);
                    }
                }
            }
            return QString();
        }

        case ColLevel: {
            LogLevel lvl = detectLogLevel(raw);
            return formatLogLevel(lvl);
        }

        case ColMessage:
            return QString::fromUtf8(raw.data(), static_cast<qsizetype>(raw.size()));

        default:
            break;
        }
    }

    return {};
}

QVariant LogModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ColLineNumber: return QCoreApplication::translate("InterfaceGrafica", "#");
        case ColTimestamp:  return QCoreApplication::translate("InterfaceGrafica", "Timestamp");
        case ColLevel:      return QCoreApplication::translate("InterfaceGrafica", "Nível");
        case ColMessage:    return QCoreApplication::translate("InterfaceGrafica", "Mensagem / Conteúdo");
        default:            break;
        }
    }
    return {};
}

void LogModel::retranslate() {
    emit headerDataChanged(Qt::Horizontal, 0, ColumnCount - 1);
}

void LogModel::setFilteredLines(std::vector<uint64_t> filteredIndices, const QString& filterDescription) {
    beginResetModel();
    m_filteredLines = std::move(filteredIndices);
    m_isFiltered = true;
    m_filterDescription = filterDescription;
    endResetModel();

    emit filterChanged(true, static_cast<int>(m_filteredLines.size()), m_filterDescription);
}

void LogModel::clearFilter() {
    if (!m_isFiltered) return;

    beginResetModel();
    m_isFiltered = false;
    m_filteredLines.clear();
    m_filterDescription.clear();
    endResetModel();

    emit filterChanged(false, 0, QString());
}

void LogModel::notifyLinesAppended(uint64_t oldLineCount, uint64_t newLineCount) {
    if (m_isFiltered) {
        // In filtered mode, appending is handled by updating the filter
        return;
    }

    if (newLineCount > oldLineCount) {
        int firstNewRow = static_cast<int>(oldLineCount);
        int lastNewRow = static_cast<int>(newLineCount - 1);
        beginInsertRows(QModelIndex(), firstNewRow, lastNewRow);
        endInsertRows();
    }
}

void LogModel::notifyIndexingFinished(uint64_t /*totalLines*/) {
    if (m_isFiltered) return;

    beginResetModel();
    endResetModel();
}

} // namespace Munin::Model
