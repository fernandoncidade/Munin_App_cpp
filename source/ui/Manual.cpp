#include "Manual.hpp"
#include "GerenciadorDialogos.hpp"
#include "ThemeManager.hpp"
#include "Win32DwmHelper.hpp"
#include "ButtonUtils.hpp"
#include "source/utils/ApplicationPathUtils.hpp"
#include "source/utils/LogManager.hpp"

#if defined(Q_OS_WIN)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <QColor>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLocale>
#include <QPointer>
#include <QPushButton>
#include <QScrollBar>
#include <QShowEvent>
#include <QSizePolicy>
#include <QSplitter>
#include <QStyle>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace Munin::UI {

ManualDialog::ManualDialog(QWidget* /*parent*/, const QString& idioma)
    : QDialog(nullptr)
    , current_lang_(idioma)
{
    try {
        setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                       Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint |
                       Qt::WindowMaximizeButtonHint);
        setModal(false);
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowIcon(get_app_icon());

        setupUI();
        atualizar_traducao(current_lang_);

        connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &ManualDialog::applyTheme);
        applyTheme(ThemeManager::instance().isDark());
    } catch (...) {
        LogManager::error(QStringLiteral("Erro ao inicializar ManualDialog"));
    }
}

void ManualDialog::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);

    auto* search_layout = new QGridLayout();
    search_layout->setContentsMargins(0, 0, 0, 0);
    search_layout->setHorizontalSpacing(8);
    search_layout->setColumnStretch(0, 0);
    search_layout->setColumnStretch(1, 1);

    search_label_ = new QLabel(this);
    search_label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    search_edit_ = new QLineEdit(this);
    search_edit_->setClearButtonEnabled(true);
    search_edit_->installEventFilter(this);

    search_previous_button_ = new QToolButton(this);
    search_next_button_ = new QToolButton(this);
    configure_tool_button(search_previous_button_, Qt::LeftArrow, QString());
    configure_tool_button(search_next_button_, Qt::RightArrow, QString());
    search_previous_button_->setEnabled(false);
    search_next_button_->setEnabled(false);

    auto* search_controls = new QHBoxLayout();
    search_controls->setContentsMargins(0, 0, 0, 0);
    search_controls->setSpacing(4);
    search_controls->addWidget(search_edit_, 1);
    search_controls->addWidget(search_previous_button_);
    search_controls->addWidget(search_next_button_);

    search_layout->addWidget(search_label_, 0, 0);
    search_layout->addLayout(search_controls, 0, 1);
    layout->addLayout(search_layout);

    splitter_ = new QSplitter(Qt::Horizontal, this);

    toc_list_ = new QListWidget(this);
    toc_list_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toc_list_->setAutoFillBackground(true);
    if (toc_list_->viewport()) {
        toc_list_->viewport()->setAutoFillBackground(true);
    }
    splitter_->addWidget(toc_list_);

    browser_ = new QTextBrowser(this);
    browser_->setOpenExternalLinks(true);
    browser_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    browser_->setAutoFillBackground(true);
    if (browser_->viewport()) {
        browser_->viewport()->setAutoFillBackground(true);
    }
    splitter_->addWidget(browser_);

    splitter_->setStretchFactor(0, 1);
    splitter_->setStretchFactor(1, 3);
    splitter_->setCollapsible(0, false);
    splitter_->setCollapsible(1, false);
    splitter_->setSizes({280, 820});

    layout->addWidget(splitter_, 1);

    auto* footer_layout = new QHBoxLayout();
    footer_layout->addStretch();

    btn_ok_ = new QPushButton(tr("OK"), this);
    configure_license_action_button(btn_ok_, 90);
    connect(btn_ok_, &QPushButton::clicked, this, &QDialog::close);
    footer_layout->addWidget(btn_ok_);
    layout->addLayout(footer_layout);

    setSizeGripEnabled(true);
    setMinimumSize(700, 480);
    resize(1000, 720);

    connect(search_edit_, &QLineEdit::textChanged, this, [this](const QString& text) {
        rebuildSearch(text);
        updateSearchButtons();
    });
    connect(search_edit_, &QLineEdit::returnPressed, this, [this]() {
        navigateSearch(1);
        updateSearchButtons();
    });
    connect(search_previous_button_, &QToolButton::clicked, this, [this]() {
        navigateSearch(-1);
        updateSearchButtons();
    });
    connect(search_next_button_, &QToolButton::clicked, this, [this]() {
        navigateSearch(1);
        updateSearchButtons();
    });

    connect(toc_list_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item == nullptr || browser_ == nullptr) return;
        const QString section_id = item->data(Qt::UserRole).toString();
        if (!section_id.isEmpty()) {
            browser_->scrollToAnchor(section_id);
        }
    });

    connect(browser_, &QTextBrowser::anchorClicked, this, [this](const QUrl& url) {
        QString section_id = url.fragment().isEmpty() ? url.toString() : url.fragment();
        if (!section_id.isEmpty()) {
            browser_->scrollToAnchor(section_id);
            for (int i = 0; i < toc_list_->count(); ++i) {
                if (toc_list_->item(i)->data(Qt::UserRole).toString() == section_id) {
                    const QSignalBlocker blocker(toc_list_);
                    toc_list_->setCurrentRow(i);
                    break;
                }
            }
        }
    });
}

bool ManualDialog::eventFilter(QObject* watched, QEvent* event) {
    if (watched == search_edit_ && event != nullptr && event->type() == QEvent::KeyPress) {
        auto* key_event = static_cast<QKeyEvent*>(event);
        if (key_event->key() == Qt::Key_Return || key_event->key() == Qt::Key_Enter) {
            navigateSearch(key_event->modifiers().testFlag(Qt::ShiftModifier) ? -1 : 1);
            updateSearchButtons();
            return true;
        }
    }

    return QDialog::eventFilter(watched, event);
}

void ManualDialog::buildSections(const QString& idioma) {
    sections_.clear();
    const bool isPt = idioma.startsWith(QStringLiteral("pt"), Qt::CaseInsensitive);

    if (isPt) {
        sections_.append({QStringLiteral("intro"), QStringLiteral("1. Visão Geral e Arquitetura"),
            QStringLiteral("<h3>1. Visão Geral e Arquitetura</h3>"
                           "<p>O <b>Munin App</b> é um leitor e analisador de logs massivos (Giga/Terabytes) de alta performance, "
                           "construído nativamente em C++20 e Qt6 para a plataforma Windows 11.</p>"
                           "<p>A engine utiliza mapeamento direto em memória virtual através de arquivos com visão de janelas deslizantes "
                           "(<b>Win32 Memory-Mapped Files — CreateFileMapping / MapViewOfFile</b>) indexadas em blocos esparsos de 64KB, "
                           "permitindo que arquivos de dezenas de gigabytes sejam abertos em frações de segundo sem estouro de memória física.</p>")});

        sections_.append({QStringLiteral("open"), QStringLiteral("2. Abertura e Monitoramento de Arquivos"),
            QStringLiteral("<h3>2. Abertura e Monitoramento de Arquivos</h3>"
                           "<p>Para abrir um arquivo de log, utilize o menu <b>Arquivo &gt; Abrir Arquivo (Ctrl+O)</b> ou o botão correspondente na barra de ferramentas.</p>"
                           "<p>O Munin App detecta automaticamente a densidade de linhas e constrói o mapa de offsets instantaneamente. "
                           "Ao ativar a opção <b>Live Tail (Ctrl+L)</b>, a aplicação monitora continuamente novos registros anexados ao final do arquivo "
                           "em tempo real através das notificações nativas do subsistema de arquivos <b>ReadDirectoryChangesW</b>.</p>")});

        sections_.append({QStringLiteral("search"), QStringLiteral("3. Motor de Busca e Filtragem SIMD"),
            QStringLiteral("<h3>3. Motor de Busca e Filtragem SIMD</h3>"
                           "<p>A barra de busca rápida (<b>Ctrl+F</b>) suporta buscas textuais exatas, diferenciação de maiúsculas/minúsculas e Expressões Regulares completas (PCRE/QRegularExpression).</p>"
                           "<p>A filtragem avançada por níveis de severidade (<b>TRACE, DEBUG, INFO, WARN, ERROR, FATAL</b>) utiliza vetorização "
                           "<b>AVX2 / SSE4.2</b> para triagem de centenas de milhões de linhas por segundo com uso nulo de alocações dinâmicas intermediárias.</p>")});

        sections_.append({QStringLiteral("timeline"), QStringLiteral("4. Timeline de Distribuição Temporal"),
            QStringLiteral("<h3>4. Timeline de Distribuição Temporal</h3>"
                           "<p>A <b>Timeline de Distribuição (Ctrl+T)</b> gera um histograma vetorial interativo da frequência de ocorrência dos eventos ao longo do tempo. "
                           "O usuário pode navegar, clicar nas faixas horárias e filtrar períodos específicos diretamente pelo gráfico.</p>")});

        sections_.append({QStringLiteral("json"), QStringLiteral("5. Inspetor JSON e Detalhes"),
            QStringLiteral("<h3>5. Inspetor JSON e Detalhes</h3>"
                           "<p>Ao selecionar qualquer linha contendo payload JSON ou estrutura tabular, o <b>Inspetor JSON (Ctrl+J)</b> disseca o objeto em uma árvore navegável com realce sintático, "
                           "busca rápida de chaves/valores e formatação identada facilitada.</p>")});

        sections_.append({QStringLiteral("themes"), QStringLiteral("6. Temas e Efeitos Visuais"),
            QStringLiteral("<h3>6. Temas e Efeitos Visuais</h3>"
                           "<p>O Munin App implementa o tema nativo e padrão do <b>Qt6</b> com suporte à transição dinâmica contínua entre modos <b>Escuro</b> e <b>Claro</b>. "
                           "A aplicação sincroniza automaticamente com as preferências visuais do Windows 11 em tempo de execução, ajustando simultaneamente a barra de títulos com efeitos imersivos <b>DWM Mica / Dark Mode</b>.</p>")});

        sections_.append({QStringLiteral("minimap"), QStringLiteral("7. Minimapa de Densidade e Navegação"),
            QStringLiteral("<h3>7. Minimapa de Densidade e Navegação</h3>"
                           "<p>A barra de rolagem lateral possui um <b>Minimapa de Densidade</b> integrado que renderiza um mapa de calor visual de todo o arquivo. "
                           "Marcadores em <b>vermelho</b> indicam erros/falhas, em <b>amarelo</b> avisos/warnings e em <b>azul/ciano</b> ocorrências da pesquisa atual. "
                           "Clique em qualquer posição do Minimapa para saltar instantaneamente para aquela região do log.</p>")});

        sections_.append({QStringLiteral("generator"), QStringLiteral("8. Gerador de Carga e Testes"),
            QStringLiteral("<h3>8. Gerador de Carga e Testes</h3>"
                           "<p>Através do menu <b>Ferramentas &gt; Gerador de Logs de Teste</b>, você pode gerar arquivos sintéticos padronizados "
                           "de 100 MB, 500 MB e 1 GB para estressar a máquina local, validar taxas de transferência e aferir a performance do hardware.</p>")});

        sections_.append({QStringLiteral("shortcuts"), QStringLiteral("9. Atalhos de Teclado e Dicas de Performance"),
            QStringLiteral("<h3>9. Atalhos de Teclado e Dicas de Performance</h3>"
                           "<p>Principais atalhos: <b>Ctrl+O</b> (Abrir Arquivo), <b>Ctrl+F</b> (Buscar), <b>Ctrl+L</b> (Live Tail), <b>Ctrl+T</b> (Timeline), <b>Ctrl+J</b> (Inspetor JSON), "
                           "<b>F3 / Shift+F3</b> (Próximo / Anterior), <b>F5</b> (Recarregar), <b>Ctrl++ / Ctrl+-</b> (Ajuste de Fonte), <b>Ctrl+0</b> (Fonte Padrão).</p>"
                           "<p><b>Dica de Performance:</b> Para máxima velocidade (&gt;3 GB/s), mantenha os arquivos de log em SSDs NVMe e aproveite a aceleração AVX2 nativa.</p>")});

        sections_.append({QStringLiteral("cli"), QStringLiteral("10. Utilitário de Linha de Comando (CLI)"),
            QStringLiteral("<h3>10. Utilitário de Linha de Comando (CLI)</h3>"
                           "<p>O <b>Munin App</b> disponibiliza o executável independente <code>munin-app-cli.exe</code>, permitindo operar a poderosa engine de processamento diretamente do Windows Terminal, PowerShell ou Prompt de Comando (CMD), sem necessidade de abrir a interface gráfica.</p>"
                           "<p><b>Principais Comandos:</b></p>"
                           "<ul>"
                           "  <li><b>info &lt;arquivo&gt;</b>: Exibe instantaneamente o tamanho formatado, total de linhas, blocos indexados e a taxa de transferência em MB/s.</li>"
                           "  <li><b>search &lt;arquivo&gt; &lt;termo&gt;</b>: Executa buscas multi-core aceleradas por SIMD. Suporta <code>--ignore-case</code> (<code>-i</code>), Expressões Regulares com <code>--regex</code> (<code>-r</code>), inversão com <code>--invert</code> (<code>-v</code>), limite de resultados com <code>--max &lt;N&gt;</code> e saída em texto ou JSON estruturado (<code>--format json</code>).</li>"
                           "  <li><b>tail &lt;arquivo&gt;</b>: Monitoramento contínuo em tempo real (estilo <i>tail -f</i>) com streaming assíncrono via kernel Win32 sem polling. Pressione <b>Ctrl+C</b> para interromper.</li>"
                           "  <li><b>bench [arquivo]</b>: Executa benchmarks automáticos avaliando taxas de mapeamento de memória, indexação SIMD e escalabilidade paralela.</li>"
                           "</ul>"
                           "<p><b>Integração Global no Sistema:</b> O pacote MSIX registra o alias <code>AppExecutionAlias</code>, permitindo executar <code>munin-app-cli</code> a partir de qualquer diretório no terminal.</p>")});
    } else {
        sections_.append({QStringLiteral("intro"), QStringLiteral("1. Overview and Architecture"),
            QStringLiteral("<h3>1. Overview and Architecture</h3>"
                           "<p><b>Munin App</b> is a high-performance massive log reader and analyzer (Giga/Terabytes), "
                           "natively crafted in C++20 and Qt6 for the Windows 11 platform.</p>"
                           "<p>The engine leverages sliding window memory mapping (<b>Win32 Memory-Mapped Files — CreateFileMapping / MapViewOfFile</b>) "
                           "indexed in sparse 64KB chunks, allowing multi-gigabyte log archives to be opened in fractions of a second with negligible RAM footprint.</p>")});

        sections_.append({QStringLiteral("open"), QStringLiteral("2. File Opening and Live Tail"),
            QStringLiteral("<h3>2. File Opening and Live Tail</h3>"
                           "<p>To open a log file, select <b>File &gt; Open File (Ctrl+O)</b> or click the toolbar button.</p>"
                           "<p>Munin App builds line offset tables on the fly. Enabling <b>Live Tail (Ctrl+L)</b> allows you to track new lines appended in real-time "
                           "via native <b>ReadDirectoryChangesW</b> kernel notifications.</p>")});

        sections_.append({QStringLiteral("search"), QStringLiteral("3. SIMD Search Engine and Filtering"),
            QStringLiteral("<h3>3. SIMD Search Engine and Filtering</h3>"
                           "<p>The instant search bar (<b>Ctrl+F</b>) supports exact literal search, case matching, and full Regular Expressions.</p>"
                           "<p>Log level filters (<b>TRACE, DEBUG, INFO, WARN, ERROR, FATAL</b>) leverage <b>AVX2 / SSE4.2</b> vectorized scans to parse millions of rows per second.</p>")});

        sections_.append({QStringLiteral("timeline"), QStringLiteral("4. Time Distribution Timeline"),
            QStringLiteral("<h3>4. Time Distribution Timeline</h3>"
                           "<p>The <b>TimeSlice Timeline (Ctrl+T)</b> generates an interactive histogram showing log frequency over time, allowing one-click filtering of peak activity periods.</p>")});

        sections_.append({QStringLiteral("json"), QStringLiteral("5. JSON Inspector and Metadata"),
            QStringLiteral("<h3>5. JSON Inspector and Metadata</h3>"
                           "<p>When clicking any line with structured JSON payloads, the <b>JSON Inspector (Ctrl+J)</b> displays an interactive, syntax-highlighted hierarchy for rapid root-cause inspection.</p>")});

        sections_.append({QStringLiteral("themes"), QStringLiteral("6. Theme Modes and Visual Effects"),
            QStringLiteral("<h3>6. Theme Modes and Visual Effects</h3>"
                           "<p>Munin App implements the native standard <b>Qt6</b> theme with seamless dynamic transitions between <b>Dark</b> and <b>Light</b> modes matching Windows 11 system preferences, integrated with native <b>DWM Mica backdrop and dark title bar</b>.</p>")});

        sections_.append({QStringLiteral("minimap"), QStringLiteral("7. Density Minimap & Navigation"),
            QStringLiteral("<h3>7. Density Minimap & Navigation</h3>"
                           "<p>The custom scrollbar incorporates an integrated <b>Density Minimap</b> rendering an event heatmap across the entire file. "
                           "<b>Red</b> markers denote errors/exceptions, <b>yellow</b> warnings, and <b>cyan/blue</b> search matches. "
                           "Click anywhere on the Minimap to jump to that section instantly.</p>")});

        sections_.append({QStringLiteral("generator"), QStringLiteral("8. Load Generator & Benchmarks"),
            QStringLiteral("<h3>8. Load Generator & Benchmarks</h3>"
                           "<p>Via <b>Tools &gt; Test Log Generator</b>, you can synthesize standardized local test archives "
                           "of 100 MB, 500 MB, and 1 GB to benchmark disk bandwidth and vector search capabilities.</p>")});

        sections_.append({QStringLiteral("shortcuts"), QStringLiteral("9. Shortcuts & Performance Tips"),
            QStringLiteral("<h3>9. Shortcuts & Performance Tips</h3>"
                           "<p>Key shortcuts: <b>Ctrl+O</b> (Open File), <b>Ctrl+F</b> (Search), <b>Ctrl+L</b> (Live Tail), <b>Ctrl+T</b> (Timeline), <b>Ctrl+J</b> (JSON Inspector), "
                           "<b>F3 / Shift+F3</b> (Next / Prev), <b>F5</b> (Reload), <b>Ctrl++ / Ctrl+-</b> (Font Scaling), <b>Ctrl+0</b> (Reset Font).</p>"
                           "<p><b>Performance Tip:</b> For maximum throughput (&gt;3 GB/s), store logs on NVMe SSDs and utilize native AVX2 vector instructions.</p>")});

        sections_.append({QStringLiteral("cli"), QStringLiteral("10. Command-Line Interface (CLI)"),
            QStringLiteral("<h3>10. Command-Line Interface (CLI)</h3>"
                           "<p><b>Munin App</b> provides the standalone <code>munin-app-cli.exe</code> executable, enabling high-speed headless log processing directly from Windows Terminal, PowerShell, or Command Prompt (CMD) without launching the graphical desktop interface.</p>"
                           "<p><b>Main Commands:</b></p>"
                           "<ul>"
                           "  <li><b>info &lt;file&gt;</b>: Instantly inspects file size, line counts, chunk index partitions, and memory-mapping throughput in MB/s.</li>"
                           "  <li><b>search &lt;file&gt; &lt;term&gt;</b>: Multi-core parallel search powered by native SIMD vector instructions. Supports <code>--ignore-case</code> (<code>-i</code>), Regular Expressions with <code>--regex</code> (<code>-r</code>), inverted matches with <code>--invert</code> (<code>-v</code>), maximum count limit with <code>--max &lt;N&gt;</code>, and formatted text or structured JSON output (<code>--format json</code>).</li>"
                           "  <li><b>tail &lt;file&gt;</b>: Follows live log files in real-time (akin to <i>tail -f</i>) using zero-overhead Windows kernel notifications without polling. Press <b>Ctrl+C</b> to stop.</li>"
                           "  <li><b>bench [file]</b>: Runs integrated benchmarks assessing memory-mapped I/O bandwidth, SIMD vectorization speed, and parallel search scaling.</li>"
                           "</ul>"
                           "<p><b>Global Terminal Integration:</b> The MSIX package registers an <code>AppExecutionAlias</code>, allowing you to invoke <code>munin-app-cli</code> from any terminal path across Windows.</p>")});
    }
}

void ManualDialog::atualizar_traducao(const QString& idioma) {
    if (!idioma.isEmpty()) {
        current_lang_ = idioma;
    } else {
        current_lang_ = QLocale().name();
    }
    const bool isPt = !current_lang_.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);

    setWindowTitle(isPt ? QStringLiteral("Manual de Instruções - Munin App - Leitor de Logs Massivos")
                        : QStringLiteral("Instruction Manual - Munin App - Massive Log Viewer"));
    if (btn_ok_ != nullptr) {
        btn_ok_->setText(QStringLiteral("OK"));
    }

    buildSections(current_lang_);

    if (toc_list_ != nullptr) {
        toc_list_->blockSignals(true);
        toc_list_->clear();
        for (const auto& sec : sections_) {
            auto* item = new QListWidgetItem(sec.title, toc_list_);
            item->setData(Qt::UserRole, sec.id);
        }
        toc_list_->blockSignals(false);
    }

    renderDocument();
    updateSearchTexts();

    if (toc_list_ != nullptr && toc_list_->count() > 0) {
        toc_list_->setCurrentRow(0);
    }
}

void ManualDialog::renderDocument() {
    if (browser_ == nullptr) return;

    const bool isDark = (palette().color(QPalette::Window).lightness() < 128);
    const QString bgColor = palette().color(QPalette::Base).name();
    const QString textColor = palette().color(QPalette::Text).name();
    const QString linkColor = palette().color(QPalette::Link).name();
    const QString borderColor = isDark ? QStringLiteral("rgba(255, 255, 255, 0.15)")
                                       : QStringLiteral("rgba(0, 0, 0, 0.12)");

    QString html;
    html += QStringLiteral(
        "<html><head><style>"
        "body { background-color: %1; color: %2; font-family: 'Segoe UI', Tahoma, sans-serif; font-size: 11pt; line-height: 1.6; margin: 12px; }"
        "p, div, span, li, b, strong { color: %2; }"
        "h1, h2, h3, h4 { color: %2; }"
        "a { color: %3; text-decoration: underline; }"
        ".section-box { margin-bottom: 24px; padding-bottom: 12px; border-bottom: 1px solid %4; }"
        "</style></head><body>"
    ).arg(bgColor, textColor, linkColor, borderColor);

    for (const auto& sec : sections_) {
        html += QStringLiteral("<a name=\"%1\" id=\"%1\"></a>").arg(sec.id);
        html += QStringLiteral("<div class=\"section-box\">");
        html += sec.htmlContent;
        html += QStringLiteral("</div>");
    }

    html += QStringLiteral("</body></html>");

    const int vPos = browser_->verticalScrollBar() ? browser_->verticalScrollBar()->value() : 0;
    const int hPos = browser_->horizontalScrollBar() ? browser_->horizontalScrollBar()->value() : 0;

    browser_->setHtml(html);

    if (browser_->document()) {
        browser_->document()->setDefaultStyleSheet(
            QStringLiteral("body, p, div, span, h1, h2, h3, h4, li { color: %1; } a { color: %2; }")
                .arg(textColor, linkColor));
    }

    if (browser_->verticalScrollBar()) browser_->verticalScrollBar()->setValue(vPos);
    if (browser_->horizontalScrollBar()) browser_->horizontalScrollBar()->setValue(hPos);

    rebuildSearch(search_edit_ != nullptr ? search_edit_->text() : QString());
    updateSearchButtons();
}

void ManualDialog::updateSearchTexts() {
    const bool isPt = current_lang_.startsWith(QStringLiteral("pt"), Qt::CaseInsensitive);

    if (search_label_) search_label_->setText(isPt ? QStringLiteral("Buscar:") : QStringLiteral("Search:"));
    if (search_edit_) search_edit_->setPlaceholderText(isPt ? QStringLiteral("Buscar texto no manual...") : QStringLiteral("Search text in manual..."));
    if (search_previous_button_) {
        const QString tip = isPt ? QStringLiteral("Resultado anterior") : QStringLiteral("Previous match");
        search_previous_button_->setToolTip(tip);
        search_previous_button_->setAccessibleName(tip);
    }
    if (search_next_button_) {
        const QString tip = isPt ? QStringLiteral("Próximo resultado") : QStringLiteral("Next match");
        search_next_button_->setToolTip(tip);
        search_next_button_->setAccessibleName(tip);
    }
}

void ManualDialog::rebuildSearch(const QString& text) {
    search_matches_.clear();
    search_current_index_ = -1;

    if (browser_ == nullptr || browser_->document() == nullptr || text.trimmed().isEmpty()) {
        updateSearchHighlights();
        return;
    }

    QTextCursor cursor(browser_->document());
    while (true) {
        cursor = browser_->document()->find(text.trimmed(), cursor);
        if (cursor.isNull()) {
            break;
        }
        search_matches_.append(cursor);
    }

    updateSearchHighlights();
}

void ManualDialog::navigateSearch(int step) {
    if (browser_ == nullptr || search_matches_.isEmpty() || step == 0) {
        return;
    }

    if (search_current_index_ < 0) {
        search_current_index_ = step < 0 ? search_matches_.size() - 1 : 0;
    } else {
        search_current_index_ =
            (search_current_index_ + step + search_matches_.size()) % search_matches_.size();
    }

    const QTextCursor cursor = search_matches_.at(search_current_index_);
    updateSearchHighlights();
    browser_->setTextCursor(cursor);
    browser_->ensureCursorVisible();
}

void ManualDialog::updateSearchButtons() {
    const bool enabled = !search_matches_.isEmpty();
    if (search_previous_button_ != nullptr) {
        search_previous_button_->setEnabled(enabled);
    }
    if (search_next_button_ != nullptr) {
        search_next_button_->setEnabled(enabled);
    }
}

void ManualDialog::updateSearchHighlights() {
    if (browser_ == nullptr) {
        return;
    }

    QList<QTextEdit::ExtraSelection> selections;
    const bool has_query = search_edit_ != nullptr && !search_edit_->text().trimmed().isEmpty();
    if (!has_query) {
        browser_->setExtraSelections(selections);
        return;
    }

    QColor highlight_color = browser_->palette().color(QPalette::Highlight);
    highlight_color.setAlpha(95);
    QColor current_color = browser_->palette().color(QPalette::Highlight);
    current_color.setAlpha(190);
    const QColor current_text_color = browser_->palette().color(QPalette::HighlightedText);

    for (int index = 0; index < search_matches_.size(); ++index) {
        QTextEdit::ExtraSelection selection;
        selection.cursor = search_matches_.at(index);
        selection.format.setBackground(index == search_current_index_ ? current_color : highlight_color);
        if (index == search_current_index_) {
            selection.format.setForeground(current_text_color);
        }
        selections.append(selection);
    }

    browser_->setExtraSelections(selections);
}

void ManualDialog::applyTheme(bool isDark) {
    Win32DwmHelper::applyTitleBarTheme(this, isDark);

    const QPalette pal = isDark ? ThemeManager::instance().createDarkPalette()
                                : ThemeManager::instance().createLightPalette();
    setPalette(pal);
    setAutoFillBackground(true);

    if (search_edit_) {
        search_edit_->setPalette(pal);
    }
    if (search_label_) {
        search_label_->setPalette(pal);
    }
    if (search_previous_button_) {
        search_previous_button_->setPalette(pal);
    }
    if (search_next_button_) {
        search_next_button_->setPalette(pal);
    }
    if (splitter_) {
        splitter_->setPalette(pal);
    }
    if (toc_list_) {
        toc_list_->setPalette(pal);
        toc_list_->setAutoFillBackground(true);
        if (toc_list_->viewport()) {
            toc_list_->viewport()->setAutoFillBackground(true);
            toc_list_->viewport()->setPalette(pal);
        }
        if (toc_list_->verticalScrollBar()) toc_list_->verticalScrollBar()->setPalette(pal);
        if (toc_list_->horizontalScrollBar()) toc_list_->horizontalScrollBar()->setPalette(pal);
    }
    if (browser_) {
        browser_->setPalette(pal);
        browser_->setAutoFillBackground(true);
        if (browser_->viewport()) {
            browser_->viewport()->setAutoFillBackground(true);
            browser_->viewport()->setPalette(pal);
        }
        if (browser_->verticalScrollBar()) browser_->verticalScrollBar()->setPalette(pal);
        if (browser_->horizontalScrollBar()) browser_->horizontalScrollBar()->setPalette(pal);
        renderDocument();
    }
    if (btn_ok_) {
        btn_ok_->setPalette(pal);
    }

    if (search_edit_ && !search_edit_->text().trimmed().isEmpty()) {
        rebuildSearch(search_edit_->text());
    } else {
        updateSearchHighlights();
    }

    update();
    repaint();
}

void ManualDialog::changeEvent(QEvent* event) {
    if (event != nullptr) {
        if (event->type() == QEvent::LanguageChange) {
            atualizar_traducao(QString());
        } else if (event->type() == QEvent::ThemeChange || event->type() == QEvent::PaletteChange) {
            applyTheme(ThemeManager::instance().isDark());
        }
    }
    QDialog::changeEvent(event);
}

bool ManualDialog::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
#if defined(Q_OS_WIN)
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        if (msg && (msg->message == WM_SETTINGCHANGE || msg->message == WM_THEMECHANGED)) {
            QTimer::singleShot(20, this, [this]() {
                applyTheme(ThemeManager::instance().isDark());
            });
        }
    }
#endif
    return QDialog::nativeEvent(eventType, message, result);
}

void ManualDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    applyTheme(ThemeManager::instance().isDark());
}

void exibir_manual(QWidget* parent, const QString& idioma) {
    GerenciadorDialogos::instance().exibir_manual(parent, idioma);
}

} // namespace Munin::UI
