#pragma once

#include <QWidget>

class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QLineEdit;
class QLabel;
class QTabWidget;
class QPushButton;

namespace Munin::UI {

class JsonInspectorWidget : public QWidget {
    Q_OBJECT

public:
    explicit JsonInspectorWidget(QWidget* parent = nullptr);

    void inspectLine(uint64_t lineIndex, uint64_t byteOffset, const QString& fullLineText);
    void clear();
    void retranslateUi();
    void applyTheme(bool isDark);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void copyFormattedJson();
    void copyRawLine();
    void filterTree(const QString& filterText);

private:
    void populateTree(const QJsonObject& obj, QTreeWidgetItem* parentItem, bool isDark);
    void populateTreeArray(const QJsonArray& arr, QTreeWidgetItem* parentItem, bool isDark);
    static QString formatJsonHtml(const QJsonDocument& doc, bool isDark);

    QLabel* m_headerLabel{nullptr};
    QLabel* m_metaLabel{nullptr};
    QLineEdit* m_filterEdit{nullptr};
    QTabWidget* m_tabWidget{nullptr};
    QTextEdit* m_formattedTextEdit{nullptr};
    QTreeWidget* m_treeWidget{nullptr};
    QTextEdit* m_rawTextEdit{nullptr};
    QPushButton* m_copyJsonBtn{nullptr};
    QPushButton* m_copyRawBtn{nullptr};

    QString m_currentRawLine;
    QString m_currentFormattedJson;
    uint64_t m_currentLineIndex{0};
    uint64_t m_currentByteOffset{0};
    bool m_hasValidJson{false};
};

} // namespace Munin::UI
