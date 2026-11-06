#pragma once

#include "../engine/ChunkIndex.hpp"
#include "../engine/MemoryMappedFile.hpp"

#include <QAbstractTableModel>
#include <QFont>
#include <QObject>
#include <QString>
#include <memory>
#include <vector>

namespace Munin::Model {

enum class LogLevel : int {
    Unknown = 0,
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

class LogModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum CustomRoles {
        RawTextRole = Qt::UserRole + 1,
        LineNumberRole,
        LogLevelRole,
        ByteOffsetRole,
        IsFilteredRole
    };

    enum Columns {
        ColLineNumber = 0,
        ColTimestamp,
        ColLevel,
        ColMessage,
        ColumnCount
    };

    explicit LogModel(QObject* parent = nullptr);
    ~LogModel() override = default;

    void setEngine(
        std::shared_ptr<Engine::MemoryMappedFile> mmap,
        std::shared_ptr<Engine::ChunkIndex> index
    );

    // QAbstractTableModel interface
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Header translation
    void retranslate();

    // Filter management
    void setFilteredLines(std::vector<uint64_t> filteredIndices, const QString& filterDescription = QString());
    void clearFilter();
    [[nodiscard]] bool isFiltered() const noexcept { return m_isFiltered; }
    [[nodiscard]] uint64_t mapRowToRealLine(int row) const noexcept;
    [[nodiscard]] QString filterDescription() const noexcept { return m_filterDescription; }

    // Live tail incremental row insertion
    void notifyLinesAppended(uint64_t oldLineCount, uint64_t newLineCount);

    // Refresh model when indexing completes
    void notifyIndexingFinished(uint64_t totalLines);

    // Fast helper to query line string_view
    [[nodiscard]] std::string_view getRawLine(uint64_t realLine) const;
    [[nodiscard]] QString getLineString(uint64_t realLine) const;

signals:
    void filterChanged(bool isFiltered, int matchCount, const QString& description);

private:
    static LogLevel detectLogLevel(std::string_view line) noexcept;
    static QString formatLogLevel(LogLevel level);

    std::shared_ptr<Engine::MemoryMappedFile> m_mmap;
    std::shared_ptr<Engine::ChunkIndex> m_index;

    bool m_isFiltered{false};
    std::vector<uint64_t> m_filteredLines;
    QString m_filterDescription;

    QFont m_monoFont;
};

} // namespace Munin::Model
