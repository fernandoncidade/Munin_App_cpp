#include "LogView.hpp"
#include "ButtonUtils.hpp"
#include "LogItemDelegate.hpp"
#include "MinimapScrollBar.hpp"
#include "StickyMenu.hpp"
#include "ThemeManager.hpp"
#include "../model/LogModel.hpp"

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QRegularExpression>
#include <QScrollBar>

namespace Munin::UI {

LogView::LogView(QWidget* parent)
    : QTableView(parent)
    , m_delegate(new LogItemDelegate(this)) {
    setItemDelegate(m_delegate);
    setShowGrid(false);
    setWordWrap(false);
    setCornerButtonEnabled(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(22);
    verticalHeader()->setMinimumSectionSize(22);

    horizontalHeader()->setHighlightSections(false);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    horizontalHeader()->setStretchLastSection(true);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &LogView::applyTheme);
    applyTheme(ThemeManager::instance().isDark());

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &LogView::onScrollChanged);
}

void LogView::applyTheme(bool isDark) {
    const QPalette pal = isDark ? ThemeManager::instance().createDarkPalette()
                                : ThemeManager::instance().createLightPalette();

    setStyleSheet(QString());
    setPalette(pal);
    if (viewport()) {
        viewport()->setPalette(pal);
        viewport()->setAutoFillBackground(true);
    }
    if (auto* hh = horizontalHeader()) {
        hh->setPalette(pal);
        if (hh->viewport()) {
            hh->viewport()->setPalette(pal);
            hh->viewport()->setAutoFillBackground(true);
        }
    }
    if (auto* vh = verticalHeader()) {
        vh->setPalette(pal);
        if (vh->viewport()) {
            vh->viewport()->setPalette(pal);
            vh->viewport()->setAutoFillBackground(true);
        }
    }
    if (auto* sbv = verticalScrollBar()) {
        sbv->setPalette(pal);
    }
    if (auto* sbh = horizontalScrollBar()) {
        sbh->setPalette(pal);
    }

    if (m_delegate) {
        m_delegate->setDarkTheme(isDark);
    }
    viewport()->update();
    update();
}

void LogView::setLogModel(Model::LogModel* model) {
    m_model = model;
    setModel(m_model);

    if (m_model) {
        setColumnWidth(Model::LogModel::ColLineNumber, 85);
        setColumnWidth(Model::LogModel::ColTimestamp, 155);
        setColumnWidth(Model::LogModel::ColLevel, 70);
    }

    if (m_minimap) {
        m_minimap->setModel(m_model);
    }
    updateMinimapRange();
}

void LogView::setMinimap(MinimapScrollBar* minimap) {
    m_minimap = minimap;
    if (m_minimap && m_model) {
        m_minimap->setModel(m_model);
    }
    updateMinimapRange();
}

void LogView::scrollToRow(int row) {
    if (!m_model || row < 0 || row >= m_model->rowCount()) return;

    QModelIndex idx = m_model->index(row, 0);
    scrollTo(idx, QAbstractItemView::PositionAtCenter);
    selectRow(row);
    updateMinimapRange();
}

void LogView::onScrollChanged(int value) {
    Q_UNUSED(value);
    updateMinimapRange();
}

void LogView::resizeEvent(QResizeEvent* event) {
    QTableView::resizeEvent(event);
    updateMinimapRange();
}

void LogView::updateMinimapRange() {
    if (!m_minimap || !m_model) return;

    int firstRow = rowAt(0);
    if (firstRow < 0) firstRow = verticalScrollBar()->value();

    int visibleCount = std::max(1, viewport()->height() / 22);
    m_minimap->setVisibleRange(firstRow, visibleCount);
}

void LogView::currentChanged(const QModelIndex& current, const QModelIndex& previous) {
    QTableView::currentChanged(current, previous);

    if (current.isValid() && m_model) {
        uint64_t realLine = m_model->mapRowToRealLine(current.row());
        QString fullLine = current.data(Model::LogModel::RawTextRole).toString();
        emit lineSelected(realLine, fullLine);
    }
}

QString LogView::extractThreadOrIp(const QString& line) {
    // Regex for IP address
    static const QRegularExpression ipRegex(QStringLiteral(R"(\b(?:\d{1,3}\.){3}\d{1,3}\b)"));
    auto ipMatch = ipRegex.match(line);
    if (ipMatch.hasMatch()) {
        return ipMatch.captured(0);
    }

    // Regex for thread ID: [thread-12], tid=402, worker-3
    static const QRegularExpression threadRegex(QStringLiteral(R"(\[(?:thread|worker|pool)[-_]?\w+\]|tid=\d+)"), QRegularExpression::CaseInsensitiveOption);
    auto thMatch = threadRegex.match(line);
    if (thMatch.hasMatch()) {
        return thMatch.captured(0);
    }

    return QString();
}

void LogView::contextMenuEvent(QContextMenuEvent* event) {
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid() || !m_model) {
        return;
    }

    QString fullLine = index.data(Model::LogModel::RawTextRole).toString();
    QString detectedEntity = extractThreadOrIp(fullLine);

    auto* menu = create_menu(QString(), this);

    if (!detectedEntity.isEmpty()) {
        QAction* filterEntityAct = menu->addAction(
            QCoreApplication::translate("InterfaceGrafica", "🎯 Filtrar por '%1'").arg(detectedEntity));
        connect(filterEntityAct, &QAction::triggered, this, [this, detectedEntity]() {
            emit filterByStringRequested(detectedEntity);
        });
    }

    // Selected text or word under cursor
    QString cellText = index.data(Qt::DisplayRole).toString();
    if (!cellText.isEmpty() && cellText != detectedEntity) {
        QString snippet = cellText.length() > 30 ? cellText.left(27) + QStringLiteral("...") : cellText;
        QAction* filterCellAct = menu->addAction(
            QCoreApplication::translate("InterfaceGrafica", "🔍 Filtrar por este valor: \"%1\"").arg(snippet));
        connect(filterCellAct, &QAction::triggered, this, [this, cellText]() {
            emit filterByStringRequested(cellText);
        });

        QAction* searchAct = menu->addAction(
            QCoreApplication::translate("InterfaceGrafica", "🔎 Buscar no arquivo: \"%1\"").arg(snippet));
        connect(searchAct, &QAction::triggered, this, [this, cellText]() {
            emit searchStringRequested(cellText);
        });
    }

    menu->addSeparator();

    QAction* copyLineAct = menu->addAction(
        QCoreApplication::translate("InterfaceGrafica", "📋 Copiar Linha Completa"));
    connect(copyLineAct, &QAction::triggered, this, [fullLine]() {
        QApplication::clipboard()->setText(fullLine);
    });

    QAction* copyCellAct = menu->addAction(
        QCoreApplication::translate("InterfaceGrafica", "📋 Copiar Este Campo"));
    connect(copyCellAct, &QAction::triggered, this, [cellText]() {
        QApplication::clipboard()->setText(cellText);
    });

    if (fullLine.contains('{') && fullLine.contains('}')) {
        menu->addSeparator();
        QAction* jsonAct = menu->addAction(
            QCoreApplication::translate("InterfaceGrafica", "🧩 Inspecionar JSON nesta Linha"));
        connect(jsonAct, &QAction::triggered, this, [this, fullLine]() {
            emit inspectJsonRequested(fullLine);
        });
    }

    if (m_model->isFiltered()) {
        menu->addSeparator();
        QAction* clearFilterAct = menu->addAction(
            QCoreApplication::translate("InterfaceGrafica", "❌ Remover Filtro Ativo"));
        connect(clearFilterAct, &QAction::triggered, this, [this]() {
            m_model->clearFilter();
            updateMinimapRange();
        });
    }

    menu->exec(event->globalPos());
    menu->deleteLater();
}

} // namespace Munin::UI
