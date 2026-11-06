#include "JsonInspectorWidget.hpp"
#include "ButtonUtils.hpp"
#include "ThemeManager.hpp"

#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QStyle>
#include <QTabWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace Munin::UI {

JsonInspectorWidget::JsonInspectorWidget(QWidget* parent)
    : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // Header bar
    auto* topLayout = new QHBoxLayout();
    m_headerLabel = new QLabel(QStringLiteral("<b>INSPETOR DE LOG & JSON</b>"), this);
    topLayout->addWidget(m_headerLabel);

    topLayout->addStretch();

    m_copyJsonBtn = new QPushButton(QCoreApplication::translate("InterfaceGrafica", "Copiar JSON"), this);
    m_copyJsonBtn->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_copyJsonBtn->setEnabled(false);
    configure_license_action_button(m_copyJsonBtn, 85);
    connect(m_copyJsonBtn, &QPushButton::clicked, this, &JsonInspectorWidget::copyFormattedJson);
    topLayout->addWidget(m_copyJsonBtn);

    m_copyRawBtn = new QPushButton(QCoreApplication::translate("InterfaceGrafica", "Copiar Raw"), this);
    m_copyRawBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    configure_license_action_button(m_copyRawBtn, 85);
    connect(m_copyRawBtn, &QPushButton::clicked, this, &JsonInspectorWidget::copyRawLine);
    topLayout->addWidget(m_copyRawBtn);

    mainLayout->addLayout(topLayout);

    // Metadata label
    m_metaLabel = new QLabel(QCoreApplication::translate("InterfaceGrafica", "Selecione uma linha para inspecionar"), this);
    mainLayout->addWidget(m_metaLabel);

    // Filter edit for tree view
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText(QCoreApplication::translate("InterfaceGrafica", "Filtrar propriedades / chaves JSON..."));
    connect(m_filterEdit, &QLineEdit::textChanged, this, &JsonInspectorWidget::filterTree);
    mainLayout->addWidget(m_filterEdit);

    // Tab Widget
    m_tabWidget = new QTabWidget(this);
    configure_document_tabs(m_tabWidget);

    // Tab 1: Formatted text edit
    m_formattedTextEdit = new QTextEdit(this);
    m_formattedTextEdit->setReadOnly(true);
    m_formattedTextEdit->setFont(QFont(QStringLiteral("Cascadia Code"), 9));
    apply_document_base_background(m_formattedTextEdit, palette());
    m_tabWidget->addTab(m_formattedTextEdit, QCoreApplication::translate("InterfaceGrafica", "JSON Formatado"));

    // Tab 2: Tree view
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({
        QCoreApplication::translate("InterfaceGrafica", "Propriedade"),
        QCoreApplication::translate("InterfaceGrafica", "Tipo"),
        QCoreApplication::translate("InterfaceGrafica", "Valor")
    });
    m_treeWidget->header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    apply_list_bg(m_treeWidget, palette().color(QPalette::Base));
    m_tabWidget->addTab(m_treeWidget, QCoreApplication::translate("InterfaceGrafica", "Árvore de Propriedades"));

    // Tab 3: Raw Line text edit
    m_rawTextEdit = new QTextEdit(this);
    m_rawTextEdit->setReadOnly(true);
    m_rawTextEdit->setFont(QFont(QStringLiteral("Cascadia Code"), 9));
    apply_document_base_background(m_rawTextEdit, palette());
    m_tabWidget->addTab(m_rawTextEdit, QCoreApplication::translate("InterfaceGrafica", "Texto Completo (Raw)"));

    mainLayout->addWidget(m_tabWidget);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &JsonInspectorWidget::applyTheme);
    applyTheme(ThemeManager::instance().isDark());
    retranslateUi();
}

void JsonInspectorWidget::applyTheme(bool isDark) {
    const QPalette pal = isDark ? ThemeManager::instance().createDarkPalette()
                                : ThemeManager::instance().createLightPalette();

    setPalette(pal);

    if (m_tabWidget) {
        m_tabWidget->setPalette(pal);
        if (m_tabWidget->tabBar()) {
            m_tabWidget->tabBar()->setPalette(pal);
        }
    }

    if (m_headerLabel) {
        m_headerLabel->setPalette(pal);
    }

    if (m_metaLabel) {
        m_metaLabel->setPalette(pal);
    }

    if (m_filterEdit) {
        m_filterEdit->setPalette(pal);
    }

    if (m_copyJsonBtn) {
        m_copyJsonBtn->setPalette(pal);
    }

    if (m_copyRawBtn) {
        m_copyRawBtn->setPalette(pal);
    }

    if (m_formattedTextEdit) {
        m_formattedTextEdit->setPalette(pal);
        if (m_formattedTextEdit->viewport()) {
            m_formattedTextEdit->viewport()->setPalette(pal);
            m_formattedTextEdit->viewport()->setAutoFillBackground(true);
        }
        if (m_formattedTextEdit->verticalScrollBar()) {
            m_formattedTextEdit->verticalScrollBar()->setPalette(pal);
        }
        if (m_formattedTextEdit->horizontalScrollBar()) {
            m_formattedTextEdit->horizontalScrollBar()->setPalette(pal);
        }
    }

    if (m_rawTextEdit) {
        m_rawTextEdit->setPalette(pal);
        if (m_rawTextEdit->viewport()) {
            m_rawTextEdit->viewport()->setPalette(pal);
            m_rawTextEdit->viewport()->setAutoFillBackground(true);
        }
        if (m_rawTextEdit->verticalScrollBar()) {
            m_rawTextEdit->verticalScrollBar()->setPalette(pal);
        }
        if (m_rawTextEdit->horizontalScrollBar()) {
            m_rawTextEdit->horizontalScrollBar()->setPalette(pal);
        }
    }

    if (m_treeWidget) {
        m_treeWidget->setPalette(pal);
        if (m_treeWidget->viewport()) {
            m_treeWidget->viewport()->setPalette(pal);
            m_treeWidget->viewport()->setAutoFillBackground(true);
        }
        if (auto* hdr = m_treeWidget->header()) {
            hdr->setPalette(pal);
            if (hdr->viewport()) {
                hdr->viewport()->setPalette(pal);
                hdr->viewport()->setAutoFillBackground(true);
            }
        }
        if (m_treeWidget->verticalScrollBar()) {
            m_treeWidget->verticalScrollBar()->setPalette(pal);
        }
        if (m_treeWidget->horizontalScrollBar()) {
            m_treeWidget->horizontalScrollBar()->setPalette(pal);
        }
    }

    // Re-render currently inspected line with new theme colors if active
    if (!m_currentRawLine.isEmpty()) {
        inspectLine(m_currentLineIndex, m_currentByteOffset, m_currentRawLine);
    }

    update();
}

void JsonInspectorWidget::retranslateUi() {
    if (m_headerLabel) {
        m_headerLabel->setText(QStringLiteral("<b>") + QCoreApplication::translate("InterfaceGrafica", "INSPETOR DE LOG & JSON") + QStringLiteral("</b>"));
    }
    if (m_copyJsonBtn) {
        m_copyJsonBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Copiar JSON"));
        const QStringList tooltipLines = {
            QCoreApplication::translate("InterfaceGrafica", "Copiar JSON formatado"),
            QCoreApplication::translate("InterfaceGrafica", "Copia a estrutura JSON formatada para a área de transferência.")
        };
        m_copyJsonBtn->setToolTip(tooltipLines.join(QLatin1Char('\n')));
        m_copyJsonBtn->setAccessibleName(m_copyJsonBtn->toolTip());
    }
    if (m_copyRawBtn) {
        m_copyRawBtn->setText(QCoreApplication::translate("InterfaceGrafica", "Copiar Raw"));
        const QStringList tooltipLines = {
            QCoreApplication::translate("InterfaceGrafica", "Copiar Linha Original"),
            QCoreApplication::translate("InterfaceGrafica", "Copia a linha bruta selecionada para a área de transferência.")
        };
        m_copyRawBtn->setToolTip(tooltipLines.join(QLatin1Char('\n')));
        m_copyRawBtn->setAccessibleName(m_copyRawBtn->toolTip());
    }
    if (m_filterEdit) {
        m_filterEdit->setPlaceholderText(QCoreApplication::translate("InterfaceGrafica", "Filtrar propriedades / chaves JSON..."));
    }
    if (m_tabWidget) {
        m_tabWidget->setTabText(0, QCoreApplication::translate("InterfaceGrafica", "JSON Formatado"));
        m_tabWidget->setTabText(1, QCoreApplication::translate("InterfaceGrafica", "Árvore de Propriedades"));
        m_tabWidget->setTabText(2, QCoreApplication::translate("InterfaceGrafica", "Texto Completo (Raw)"));
    }
    if (m_treeWidget) {
        m_treeWidget->setHeaderLabels({
            QCoreApplication::translate("InterfaceGrafica", "Propriedade"),
            QCoreApplication::translate("InterfaceGrafica", "Tipo"),
            QCoreApplication::translate("InterfaceGrafica", "Valor")
        });
    }
    if (m_currentRawLine.isEmpty()) {
        if (m_metaLabel) {
            m_metaLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Selecione uma linha para inspecionar"));
        }
    }
}

void JsonInspectorWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void JsonInspectorWidget::clear() {
    m_currentRawLine.clear();
    m_currentFormattedJson.clear();
    m_currentLineIndex = 0;
    m_currentByteOffset = 0;
    m_hasValidJson = false;
    m_metaLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Nenhuma linha selecionada"));
    m_formattedTextEdit->clear();
    m_treeWidget->clear();
    m_rawTextEdit->clear();
    m_copyJsonBtn->setEnabled(false);
}

void JsonInspectorWidget::inspectLine(uint64_t lineIndex, uint64_t byteOffset, const QString& fullLineText) {
    m_currentLineIndex = lineIndex;
    m_currentByteOffset = byteOffset;
    m_currentRawLine = fullLineText;
    m_treeWidget->clear();
    m_formattedTextEdit->clear();
    m_rawTextEdit->setPlainText(fullLineText);

    double offsetMB = static_cast<double>(byteOffset) / (1024.0 * 1024.0);

    // Look for JSON opening and closing
    int firstBrace = fullLineText.indexOf('{');
    int lastBrace = fullLineText.lastIndexOf('}');
    int firstBracket = fullLineText.indexOf('[');
    int lastBracket = fullLineText.lastIndexOf(']');

    bool candidateObject = (firstBrace >= 0 && lastBrace > firstBrace);
    bool candidateArray = (firstBracket >= 0 && lastBracket > firstBracket);

    QJsonDocument doc;
    QJsonParseError err{};

    if (candidateObject && (!candidateArray || firstBrace <= firstBracket)) {
        QString jsonStr = fullLineText.mid(firstBrace, lastBrace - firstBrace + 1);
        doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &err);
    } else if (candidateArray) {
        QString jsonStr = fullLineText.mid(firstBracket, lastBracket - firstBracket + 1);
        doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &err);
    }

    const bool isDark = ThemeManager::instance().isDark();

    if (!doc.isNull() && err.error == QJsonParseError::NoError) {
        m_hasValidJson = true;
        m_currentFormattedJson = QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
        m_copyJsonBtn->setEnabled(true);

        int propCount = doc.isObject() ? doc.object().size() : (doc.isArray() ? doc.array().size() : 1);
        m_metaLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Linha %1 | Offset: %2 MB | JSON Válido (%3 propriedades)")
                                 .arg(lineIndex + 1)
                                 .arg(offsetMB, 0, 'f', 2)
                                 .arg(propCount));

        m_formattedTextEdit->setHtml(formatJsonHtml(doc, isDark));

        if (doc.isObject()) {
            populateTree(doc.object(), nullptr, isDark);
        } else if (doc.isArray()) {
            populateTreeArray(doc.array(), nullptr, isDark);
        }

        m_treeWidget->expandAll();
        m_treeWidget->resizeColumnToContents(0);
        m_treeWidget->resizeColumnToContents(1);

        if (!m_filterEdit->text().isEmpty()) {
            filterTree(m_filterEdit->text());
        }
    } else {
        m_hasValidJson = false;
        m_currentFormattedJson.clear();
        m_copyJsonBtn->setEnabled(false);

        m_metaLabel->setText(QCoreApplication::translate("InterfaceGrafica", "Linha %1 | Offset: %2 MB | Texto Simples (Sem JSON detectado)")
                                 .arg(lineIndex + 1)
                                 .arg(offsetMB, 0, 'f', 2));

        m_formattedTextEdit->setPlainText(fullLineText);
    }
}

void JsonInspectorWidget::copyFormattedJson() {
    if (m_hasValidJson && !m_currentFormattedJson.isEmpty()) {
        QApplication::clipboard()->setText(m_currentFormattedJson);
    }
}

void JsonInspectorWidget::copyRawLine() {
    if (!m_currentRawLine.isEmpty()) {
        QApplication::clipboard()->setText(m_currentRawLine);
    }
}

void JsonInspectorWidget::populateTree(const QJsonObject& obj, QTreeWidgetItem* parentItem, bool isDark) {
    const QColor textCol = isDark ? QColor(235, 235, 235) : QColor(20, 20, 20);
    const QColor typeCol = isDark ? QColor(161, 161, 170) : QColor(100, 116, 139);
    const QColor strCol = isDark ? QColor(253, 224, 71) : QColor(180, 83, 9);
    const QColor numCol = isDark ? QColor(134, 239, 172) : QColor(21, 128, 61);
    const QColor boolCol = isDark ? QColor(244, 114, 182) : QColor(190, 24, 93);
    const QColor nullCol = isDark ? QColor(148, 163, 184) : QColor(100, 116, 139);

    for (auto it = obj.begin(); it != obj.end(); ++it) {
        auto* item = new QTreeWidgetItem();
        item->setText(0, it.key());
        item->setForeground(0, textCol);

        const QJsonValue val = it.value();
        if (val.isObject()) {
            item->setText(1, QStringLiteral("Object"));
            item->setForeground(1, typeCol);
            item->setText(2, QStringLiteral("{...}"));
            item->setForeground(2, textCol);
            populateTree(val.toObject(), item, isDark);
        } else if (val.isArray()) {
            item->setText(1, QStringLiteral("Array"));
            item->setForeground(1, typeCol);
            item->setText(2, QStringLiteral("[%1 items]").arg(val.toArray().size()));
            item->setForeground(2, textCol);
            populateTreeArray(val.toArray(), item, isDark);
        } else if (val.isString()) {
            item->setText(1, QStringLiteral("String"));
            item->setForeground(1, typeCol);
            item->setText(2, QStringLiteral("\"%1\"").arg(val.toString()));
            item->setForeground(2, strCol);
        } else if (val.isDouble()) {
            item->setText(1, QStringLiteral("Number"));
            item->setForeground(1, typeCol);
            item->setText(2, QString::number(val.toDouble()));
            item->setForeground(2, numCol);
        } else if (val.isBool()) {
            item->setText(1, QStringLiteral("Boolean"));
            item->setForeground(1, typeCol);
            item->setText(2, val.toBool() ? QStringLiteral("true") : QStringLiteral("false"));
            item->setForeground(2, boolCol);
        } else if (val.isNull()) {
            item->setText(1, QStringLiteral("Null"));
            item->setForeground(1, typeCol);
            item->setText(2, QStringLiteral("null"));
            item->setForeground(2, nullCol);
        }

        if (parentItem) {
            parentItem->addChild(item);
        } else {
            m_treeWidget->addTopLevelItem(item);
        }
    }
}

void JsonInspectorWidget::populateTreeArray(const QJsonArray& arr, QTreeWidgetItem* parentItem, bool isDark) {
    const QColor textCol = isDark ? QColor(235, 235, 235) : QColor(20, 20, 20);
    const QColor typeCol = isDark ? QColor(161, 161, 170) : QColor(100, 116, 139);
    const QColor strCol = isDark ? QColor(253, 224, 71) : QColor(180, 83, 9);
    const QColor numCol = isDark ? QColor(134, 239, 172) : QColor(21, 128, 61);
    const QColor boolCol = isDark ? QColor(244, 114, 182) : QColor(190, 24, 93);

    for (int i = 0; i < arr.size(); ++i) {
        auto* item = new QTreeWidgetItem();
        item->setText(0, QStringLiteral("[%1]").arg(i));
        item->setForeground(0, textCol);

        const QJsonValue val = arr[i];
        if (val.isObject()) {
            item->setText(1, QStringLiteral("Object"));
            item->setForeground(1, typeCol);
            item->setText(2, QStringLiteral("{...}"));
            item->setForeground(2, textCol);
            populateTree(val.toObject(), item, isDark);
        } else if (val.isArray()) {
            item->setText(1, QStringLiteral("Array"));
            item->setForeground(1, typeCol);
            item->setText(2, QStringLiteral("[%1 items]").arg(val.toArray().size()));
            item->setForeground(2, textCol);
            populateTreeArray(val.toArray(), item, isDark);
        } else if (val.isString()) {
            item->setText(1, QStringLiteral("String"));
            item->setForeground(1, typeCol);
            item->setText(2, QStringLiteral("\"%1\"").arg(val.toString()));
            item->setForeground(2, strCol);
        } else if (val.isDouble()) {
            item->setText(1, QStringLiteral("Number"));
            item->setForeground(1, typeCol);
            item->setText(2, QString::number(val.toDouble()));
            item->setForeground(2, numCol);
        } else if (val.isBool()) {
            item->setText(1, QStringLiteral("Boolean"));
            item->setForeground(1, typeCol);
            item->setText(2, val.toBool() ? QStringLiteral("true") : QStringLiteral("false"));
            item->setForeground(2, boolCol);
        }

        if (parentItem) {
            parentItem->addChild(item);
        } else {
            m_treeWidget->addTopLevelItem(item);
        }
    }
}

void JsonInspectorWidget::filterTree(const QString& filterText) {
    if (filterText.isEmpty()) {
        for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
            auto* item = m_treeWidget->topLevelItem(i);
            item->setHidden(false);
        }
        return;
    }

    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        auto* item = m_treeWidget->topLevelItem(i);
        bool match = item->text(0).contains(filterText, Qt::CaseInsensitive) ||
                     item->text(2).contains(filterText, Qt::CaseInsensitive);
        item->setHidden(!match);
    }
}

QString JsonInspectorWidget::formatJsonHtml(const QJsonDocument& doc, bool isDark) {
    QString raw = QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
    QString escaped = raw.toHtmlEscaped();

    const QString textColor = isDark ? QStringLiteral("#ebebeb") : QStringLiteral("#141414");
    const QString keyColor = isDark ? QStringLiteral("#38bdf8") : QStringLiteral("#0284c7");
    const QString strColor = isDark ? QStringLiteral("#fde047") : QStringLiteral("#b45309");
    const QString numColor = isDark ? QStringLiteral("#4ade80") : QStringLiteral("#16a34a");
    const QString boolColor = isDark ? QStringLiteral("#f472b6") : QStringLiteral("#db2777");

    // Color keys: "key":
    static const QRegularExpression keyRegex(QStringLiteral(R"(&quot;([^&]+)&quot;:)"));
    escaped.replace(keyRegex, QStringLiteral("<span style='color:%1;font-weight:bold;'>&quot;\\1&quot;</span>:").arg(keyColor));

    // Color string values: : "value"
    static const QRegularExpression strValRegex(QStringLiteral(R"(:\s*&quot;([^&]*)&quot;)"));
    escaped.replace(strValRegex, QStringLiteral(": <span style='color:%1;'>&quot;\\1&quot;</span>").arg(strColor));

    // Color numbers
    static const QRegularExpression numRegex(QStringLiteral(R"(:\s*(-?\d+\.?\d*))"));
    escaped.replace(numRegex, QStringLiteral(": <span style='color:%1;'>\\1</span>").arg(numColor));

    // Color booleans & null
    static const QRegularExpression boolRegex(QStringLiteral(R"(:\s*(true|false|null))"));
    escaped.replace(boolRegex, QStringLiteral(": <span style='color:%1;font-weight:bold;'>\\1</span>").arg(boolColor));

    return QStringLiteral("<pre style='margin:0;font-family:Consolas,monospace;font-size:12px;color:%1;'>").arg(textColor)
           + escaped + QStringLiteral("</pre>");
}

} // namespace Munin::UI
