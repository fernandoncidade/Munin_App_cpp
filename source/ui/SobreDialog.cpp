#include "SobreDialog.hpp"
#include "ThemeManager.hpp"
#include "Win32DwmHelper.hpp"
#include "GerenciadorDialogos.hpp"
#include "OpcoesSobre.hpp"
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

#include <algorithm>
#include <functional>
#include <QColor>
#include <QDesktopServices>
#include <QEvent>
#include <QFontMetrics>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPalette>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScrollBar>
#include <QShowEvent>
#include <QSizePolicy>
#include <QStyle>
#include <QTabBar>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QtMath>

namespace Munin::UI {
namespace {

QString build_license_doc_href(const QString& doc_id) {
    return QStringLiteral("license-doc://") + doc_id;
}

QString render_clickable_text_body(const QString& content, const QHash<QString, QString>& displayed_path_to_doc_id) {
    if (content.isEmpty()) {
        return QStringLiteral("<div style=\"white-space: pre-wrap;\"></div>");
    }

    struct LinkMatch {
        int start = 0;
        int length = 0;
        QString href;
        QString label;
    };

    QVector<LinkMatch> matches;

    static const QRegularExpression url_regex(QStringLiteral(R"(https?://[^\s<>"']+)"));
    const auto url_matches = url_regex.globalMatch(content);
    auto it_url = url_matches;
    while (it_url.hasNext()) {
        const auto match = it_url.next();
        const QString url = match.captured(0);
        LinkMatch link_match;
        link_match.start = match.capturedStart(0);
        link_match.length = match.capturedLength(0);
        link_match.href = url;
        link_match.label = url;
        matches.push_back(link_match);
    }

    for (auto it = displayed_path_to_doc_id.constBegin(); it != displayed_path_to_doc_id.constEnd(); ++it) {
        const QString& displayed_path = it.key();
        const QString& doc_id = it.value();
        int from = 0;
        while ((from = content.indexOf(displayed_path, from)) >= 0) {
            LinkMatch link_match;
            link_match.start = from;
            link_match.length = displayed_path.size();
            link_match.href = build_license_doc_href(doc_id);
            link_match.label = displayed_path;
            matches.push_back(link_match);
            from += displayed_path.size();
        }
    }

    std::sort(matches.begin(), matches.end(), [](const LinkMatch& lhs, const LinkMatch& rhs) {
        if (lhs.start != rhs.start) {
            return lhs.start < rhs.start;
        }
        return lhs.length > rhs.length;
    });

    QString html = QStringLiteral("<div style=\"white-space: pre-wrap;\">");
    int current_pos = 0;
    for (const LinkMatch& match : matches) {
        if (match.start < current_pos) {
            continue;
        }

        html += content.mid(current_pos, match.start - current_pos).toHtmlEscaped();
        html += QStringLiteral("<a href=\"") + match.href.toHtmlEscaped() + QStringLiteral("\">") +
                match.label.toHtmlEscaped() + QStringLiteral("</a>");
        current_pos = match.start + match.length;
    }

    html += content.mid(current_pos).toHtmlEscaped();
    html += QStringLiteral("</div>");
    return html;
}

QString render_clickable_text_document(const QString& content,
                                       const QHash<QString, QString>& displayed_path_to_doc_id,
                                       const QPalette& pal = QPalette()) {
    const QPalette actualPal = pal.color(QPalette::Base).isValid() ? pal : (qApp ? qApp->palette() : QPalette());
    const QString bgColor = actualPal.color(QPalette::Base).name();
    const QString textColor = actualPal.color(QPalette::Text).name();
    const QString linkColor = actualPal.color(QPalette::Link).name();

    return QStringLiteral(
        "<html><head><style>"
        "body { background-color: %1; color: %2; font-family: 'Segoe UI', Tahoma, sans-serif; font-size: 10pt; line-height: 1.5; margin: 8px; }"
        "p, div, span, li, b, strong { color: %2; }"
        "h1, h2, h3, h4 { color: %2; }"
        "a { color: %3; text-decoration: underline; }"
        "</style></head><body>%4</body></html>"
    ).arg(bgColor, textColor, linkColor, render_clickable_text_body(content, displayed_path_to_doc_id));
}

QString montar_html_licencas(const QString& licencas,
                             const QString& site_oficial_text,
                             const QString& sites_licencas,
                             const QString& info_not_available_text,
                             const QHash<QString, QString>& displayed_path_to_doc_id,
                             const QPalette& pal = QPalette()) {
    const QPalette actualPal = pal.color(QPalette::Base).isValid() ? pal : (qApp ? qApp->palette() : QPalette());
    const QString bgColor = actualPal.color(QPalette::Base).name();
    const QString textColor = actualPal.color(QPalette::Text).name();
    const QString linkColor = actualPal.color(QPalette::Link).name();

    QString texto_html = QStringLiteral(
        "<html><head><style>"
        "body { background-color: %1; color: %2; font-family: 'Segoe UI', Tahoma, sans-serif; font-size: 10pt; line-height: 1.5; margin: 8px; }"
        "p, div, span, li, b, strong { color: %2; }"
        "h1, h2, h3, h4 { color: %2; }"
        "a { color: %3; text-decoration: underline; }"
        "</style></head><body>"
    ).arg(bgColor, textColor, linkColor);

    if (licencas.isEmpty()) {
        texto_html += QStringLiteral("<p>%1.</p>").arg(info_not_available_text.toHtmlEscaped());
    } else {
        texto_html += render_clickable_text_body(licencas, displayed_path_to_doc_id);
    }

    texto_html += QStringLiteral("<br><br><h3>") + site_oficial_text.toHtmlEscaped() + QStringLiteral("</h3><ul>");

    const QStringList sites = sites_licencas.split('\n', Qt::SkipEmptyParts);
    for (const QString& site : sites) {
        const QString site_limpo = site.trimmed();
        if (!site_limpo.isEmpty()) {
            texto_html += QStringLiteral("<li><a href=\"") + site_limpo.toHtmlEscaped() + QStringLiteral("\">") +
                          site_limpo.toHtmlEscaped() + QStringLiteral("</a></li>");
        }
    }

    texto_html += QStringLiteral("</ul></body></html>");
    return texto_html;
}

struct AboutSearchMatch {
    QTextBrowser* browser = nullptr;
    int start = -1;
    int length = 0;
};

QVector<AboutSearchMatch> browser_search_matches(QTextBrowser* browser, const QString& query) {
    QVector<AboutSearchMatch> matches;
    if (browser == nullptr || browser->document() == nullptr || query.trimmed().isEmpty()) {
        return matches;
    }

    QTextCursor cursor(browser->document());
    while (true) {
        cursor = browser->document()->find(query.trimmed(), cursor);
        if (cursor.isNull()) {
            break;
        }
        matches.append({browser, cursor.selectionStart(), cursor.selectionEnd() - cursor.selectionStart()});
    }
    return matches;
}

QVector<AboutSearchMatch> collect_about_search_matches(QWidget* root, const QString& query, bool visible_only) {
    QVector<AboutSearchMatch> matches;
    if (root == nullptr || query.trimmed().isEmpty()) {
        return matches;
    }

    auto append_browser = [&](QTextBrowser* browser) {
        if (browser == nullptr || (visible_only && !browser->isVisibleTo(root))) {
            return;
        }
        matches += browser_search_matches(browser, query);
    };

    append_browser(qobject_cast<QTextBrowser*>(root));
    for (QTextBrowser* browser : root->findChildren<QTextBrowser*>()) {
        append_browser(browser);
    }

    return matches;
}

void apply_about_search_highlights(QWidget* root,
                                   const QString& query,
                                   QTextBrowser* current_browser,
                                   int current_start) {
    if (root == nullptr) {
        return;
    }

    const QString trimmed = query.trimmed();
    const QColor highlight = root->palette().color(QPalette::Highlight);
    const QColor highlighted_text = root->palette().color(QPalette::HighlightedText);

    auto apply_browser = [&](QTextBrowser* browser) {
        if (browser == nullptr) {
            return;
        }

        QList<QTextEdit::ExtraSelection> selections;
        const QVector<AboutSearchMatch> matches = browser_search_matches(browser, trimmed);
        for (const AboutSearchMatch& match : matches) {
            QTextCursor cursor(browser->document());
            cursor.setPosition(match.start);
            cursor.setPosition(match.start + match.length, QTextCursor::KeepAnchor);

            QColor background = highlight;
            background.setAlpha(browser == current_browser && match.start == current_start ? 190 : 95);

            QTextEdit::ExtraSelection selection;
            selection.cursor = cursor;
            selection.format.setBackground(background);
            if (browser == current_browser && match.start == current_start) {
                selection.format.setForeground(highlighted_text);
            }
            selections.append(selection);
        }
        browser->setExtraSelections(selections);
    };

    apply_browser(qobject_cast<QTextBrowser*>(root));
    for (QTextBrowser* browser : root->findChildren<QTextBrowser*>()) {
        apply_browser(browser);
    }
}

int find_about_search_match_index(const QVector<AboutSearchMatch>& matches, QTextBrowser* browser, int start) {
    for (int index = 0; index < matches.size(); ++index) {
        if (matches.at(index).browser == browser && matches.at(index).start == start) {
            return index;
        }
    }
    return -1;
}

void scroll_about_search_match_to_view(const AboutSearchMatch& match) {
    if (match.browser == nullptr || match.browser->document() == nullptr) {
        return;
    }

    QTextCursor cursor(match.browser->document());
    cursor.setPosition(match.start);
    cursor.setPosition(match.start + match.length, QTextCursor::KeepAnchor);
    match.browser->setTextCursor(cursor);
    match.browser->ensureCursorVisible();
}

} // namespace

SobreDialog::SobreDialog(QWidget* /*parent*/,
                         const QString& titulo,
                         const QString& texto_fixo,
                         const QString& texto_history,
                         const QString& detalhes,
                         const QString& licencas,
                         const QString& sites_licencas,
                         const QString& show_history_text,
                         const QString& hide_history_text,
                         const QString& show_details_text,
                         const QString& hide_details_text,
                         const QString& show_licenses_text,
                         const QString& hide_licenses_text,
                         const QString& ok_text,
                         const QString& site_oficial_text,
                         const QString& avisos,
                         const QString& show_notices_text,
                         const QString& hide_notices_text,
                         const QString& privacy_policy,
                         const QString& show_privacy_policy_text,
                         const QString& hide_privacy_policy_text,
                         const QString& info_not_available_text,
                         const QString& release_notes,
                         const QString& show_release_notes_text,
                         const QString& hide_release_notes_text,
                         const QString& clc,
                         const QString& copyright)
    : QDialog(nullptr),
      info_not_available_text_(info_not_available_text),
      sites_licencas_(sites_licencas),
      site_oficial_text_(site_oficial_text) {
    try {
        setWindowTitle(titulo);
        setWindowIcon(get_app_icon());
        setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                       Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
        setModal(false);
        setAttribute(Qt::WA_DeleteOnClose);

        auto* layout = new QVBoxLayout(this);
        layout->setSpacing(6);
        const int search_top_margin = std::max(0, fontMetrics().lineSpacing() - layout->spacing());

        auto* header_widget = new QWidget(this);
        auto* header_layout = new QVBoxLayout(header_widget);
        header_layout->setContentsMargins(0, 0, 0, 0);
        header_layout->setSpacing(0);

        fixed_label_ = new QLabel(texto_fixo, this);
        fixed_label_->setTextFormat(Qt::RichText);
        fixed_label_->setWordWrap(true);
        fixed_label_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        fixed_label_->setContentsMargins(0, 0, 0, 0);
        fixed_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        header_layout->addWidget(fixed_label_);

        header_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        layout->addWidget(header_widget);

        auto* search_layout = new QGridLayout();
        search_layout->setContentsMargins(0, search_top_margin, 0, 6);
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
        search_previous_button_->setArrowType(Qt::LeftArrow);
        search_next_button_->setArrowType(Qt::RightArrow);
        search_previous_button_->setAutoRaise(true);
        search_next_button_->setAutoRaise(true);
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

        tabs_ = new QTabWidget(this);
        tabs_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        configure_document_tabs(tabs_);

        // 1. Aba Sobre (source/assets/ABOUT/ABOUT_*.txt)
        about_browser_ = new QTextBrowser(this);
        detalhes_browser_ = about_browser_;
        configure_browser(about_browser_);
        apply_text_or_placeholder(about_browser_, detalhes, info_not_available_text);
        tabs_->addTab(about_browser_, show_details_text.isEmpty() ? QStringLiteral("Sobre") : show_details_text);
        tab_labels_.push_back({show_details_text.isEmpty() ? QStringLiteral("Sobre") : show_details_text,
                               hide_details_text.isEmpty() ? QStringLiteral("Ocultar Sobre") : hide_details_text});

        // 2. Aba Histórico (source/assets/ABOUT/History_APP_*.txt)
        history_browser_ = new QTextBrowser(this);
        configure_browser(history_browser_);
        apply_text_or_placeholder(history_browser_, texto_history, info_not_available_text);
        tabs_->addTab(history_browser_, show_history_text.isEmpty() ? QStringLiteral("Histórico") : show_history_text);
        tab_labels_.push_back({show_history_text.isEmpty() ? QStringLiteral("Histórico") : show_history_text,
                               hide_history_text.isEmpty() ? QStringLiteral("Ocultar Histórico") : hide_history_text});

        // 3. Aba EULA (source/assets/EULA/EULA_*.txt)
        eula_browser_ = new QTextBrowser(this);
        configure_browser(eula_browser_);
        apply_text_or_placeholder(eula_browser_, licencas, info_not_available_text);
        tabs_->addTab(eula_browser_, QStringLiteral("EULA"));
        tab_labels_.push_back({QStringLiteral("EULA"), QStringLiteral("Ocultar EULA")});

        // 4. Aba Contrato Comercial (source/assets/CLC/CLC_*.txt)
        clc_browser_ = new QTextBrowser(this);
        configure_browser(clc_browser_);
        apply_text_or_placeholder(clc_browser_, clc.isEmpty() ? OpcoesSobre::CLC_TEXT_PT_BR : clc, info_not_available_text);
        tabs_->addTab(clc_browser_, QStringLiteral("Contrato Comercial"));
        tab_labels_.push_back({QStringLiteral("Contrato Comercial"), QStringLiteral("Ocultar Contrato")});

        // 5. Aba Copyright e Marcas (source/assets/COPYRIGHT/*)
        copyright_browser_ = new QTextBrowser(this);
        configure_browser(copyright_browser_);
        apply_text_or_placeholder(copyright_browser_, copyright.isEmpty() ? OpcoesSobre::COPYRIGHT_TEXT_PT_BR : copyright, info_not_available_text);
        tabs_->addTab(copyright_browser_, QStringLiteral("Copyright e Marcas"));
        tab_labels_.push_back({QStringLiteral("Copyright e Marcas"), QStringLiteral("Ocultar Copyright")});

        // 6. Aba Licenças de Terceiros (source/assets/LICENSES/*)
        licencas_browser_ = new QTextBrowser(this);
        configure_browser(licencas_browser_);
        licencas_browser_->setHtml(montar_html_licencas(
            OpcoesSobre::LICENSE_OWNER, site_oficial_text_.isEmpty() ? QStringLiteral("Site Oficial") : site_oficial_text_,
            sites_licencas_, info_not_available_text_, displayed_path_to_doc_id_));
        tabs_->addTab(licencas_browser_, show_licenses_text.isEmpty() ? QStringLiteral("Licenças") : show_licenses_text);
        tab_labels_.push_back({show_licenses_text.isEmpty() ? QStringLiteral("Licenças") : show_licenses_text,
                               hide_licenses_text.isEmpty() ? QStringLiteral("Ocultar Licenças") : hide_licenses_text});

        // 7. Aba Avisos (source/assets/NOTICES/NOTICE_*.txt)
        avisos_browser_ = new QTextBrowser(this);
        configure_browser(avisos_browser_);
        apply_text_or_placeholder(avisos_browser_, avisos, info_not_available_text);
        tabs_->addTab(avisos_browser_, show_notices_text.isEmpty() ? QStringLiteral("Avisos") : show_notices_text);
        tab_labels_.push_back({show_notices_text.isEmpty() ? QStringLiteral("Avisos") : show_notices_text,
                               hide_notices_text.isEmpty() ? QStringLiteral("Ocultar Avisos") : hide_notices_text});

        // 8. Aba Política de Privacidade (source/assets/PRIVACY_POLICY/Privacy_Policy_*.txt)
        privacidade_browser_ = new QTextBrowser(this);
        configure_browser(privacidade_browser_);
        apply_text_or_placeholder(privacidade_browser_, privacy_policy, info_not_available_text);
        tabs_->addTab(privacidade_browser_,
                      show_privacy_policy_text.isEmpty() ? QStringLiteral("Privacidade") : show_privacy_policy_text);
        tab_labels_.push_back({show_privacy_policy_text.isEmpty() ? QStringLiteral("Privacidade") : show_privacy_policy_text,
                               hide_privacy_policy_text.isEmpty() ? QStringLiteral("Ocultar Privacidade") : hide_privacy_policy_text});

        // 9. Aba Notas de Lançamento (source/assets/RELEASE/RELEASE NOTES_*.txt)
        release_notes_browser_ = new QTextBrowser(this);
        configure_browser(release_notes_browser_);
        apply_text_or_placeholder(release_notes_browser_, release_notes, info_not_available_text);
        tabs_->addTab(release_notes_browser_,
                      show_release_notes_text.isEmpty() ? QStringLiteral("Notas de Lançamento") : show_release_notes_text);
        tab_labels_.push_back({show_release_notes_text.isEmpty() ? QStringLiteral("Notas de Lançamento") : show_release_notes_text,
                               hide_release_notes_text.isEmpty() ? QStringLiteral("Ocultar Notas") : hide_release_notes_text});

        layout->addWidget(tabs_);

        auto* button_layout = new QHBoxLayout();
        ok_button_ = new QPushButton(ok_text, this);
        configure_license_action_button(ok_button_, 90);
        connect(ok_button_, &QPushButton::clicked, this, &QDialog::accept);
        button_layout->addStretch(1);
        button_layout->addWidget(ok_button_);
        layout->addLayout(button_layout);

        setMinimumSize(720, 520);
        resize(880, 620);

        connect(tabs_, &QTabWidget::currentChanged, this, &SobreDialog::update_tab_labels);
        connect(tabs_, &QTabWidget::currentChanged, this, [this](int) {
            reset_about_search_position();
            apply_about_search();
            update_about_search_buttons();
        });
        connect(search_edit_, &QLineEdit::textChanged, this, [this]() {
            reset_about_search_position();
            apply_about_search();
            update_about_search_buttons();
        });
        connect(search_edit_, &QLineEdit::returnPressed, this, [this]() {
            navigate_about_search(1);
        });
        connect(search_previous_button_, &QToolButton::clicked, this, [this]() {
            navigate_about_search(-1);
        });
        connect(search_next_button_, &QToolButton::clicked, this, [this]() {
            navigate_about_search(1);
        });

        update_search_texts();
        update_tab_labels(tabs_->currentIndex());

        connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &SobreDialog::applyTheme);
        applyTheme(ThemeManager::instance().isDark());

        QTimer::singleShot(0, this, [this]() { refresh_fixed_label_geometry(); });
    } catch (...) {
        LogManager::error(QStringLiteral("Erro ao criar SobreDialog"));
    }
}

void SobreDialog::configure_browser(QTextBrowser* browser) {
    if (browser == nullptr) return;
    browser->setReadOnly(true);
    browser->setOpenLinks(false);
    browser->setOpenExternalLinks(false);
    browser->setAutoFillBackground(true);
    if (browser->viewport()) {
        browser->viewport()->setAutoFillBackground(true);
    }
    connect(browser, &QTextBrowser::anchorClicked, this, &SobreDialog::handle_anchor_click);
}

void SobreDialog::apply_text_or_placeholder(QTextBrowser* browser,
                                            const QString& content,
                                            const QString& info_not_available_text,
                                            const QPalette& pal) {
    if (browser == nullptr) return;
    const QPalette actualPal = pal.color(QPalette::Base).isValid() ? pal : (qApp ? qApp->palette() : QPalette());
    if (!content.isEmpty()) {
        browser->setHtml(render_clickable_text_document(content, displayed_path_to_doc_id_, actualPal));
    } else {
        const QString bgColor = actualPal.color(QPalette::Base).name();
        const QString textColor = actualPal.color(QPalette::Text).name();
        browser->setHtml(QStringLiteral(
            "<html><head><style>"
            "body { background-color: %1; color: %2; font-family: 'Segoe UI', Tahoma, sans-serif; font-size: 10pt; line-height: 1.5; margin: 8px; }"
            "p { color: %2; }"
            "</style></head><body><p>%3.</p></body></html>"
        ).arg(bgColor, textColor, info_not_available_text.toHtmlEscaped()));
    }
}

void SobreDialog::register_license_document(const QString& doc_id,
                                            const QString& displayed_path,
                                            const QString& license_title,
                                            const QString& content) {
    if (doc_id.isEmpty() || displayed_path.isEmpty() || content.isEmpty()) return;
    displayed_path_to_doc_id_.insert(displayed_path, doc_id);
    license_document_titles_.insert(doc_id, license_title);
    license_document_contents_.insert(doc_id, content);
}

void SobreDialog::handle_anchor_click(const QUrl& url) {
    if (!url.isValid()) return;

    if (url.scheme() == QStringLiteral("license-doc")) {
        QString doc_id = url.host();
        if (doc_id.isEmpty()) {
            doc_id = url.path();
            if (doc_id.startsWith('/')) doc_id.remove(0, 1);
        }
        open_license_document(doc_id);
        return;
    }

    if (!QDesktopServices::openUrl(url)) {
        LogManager::error(QStringLiteral("Erro ao abrir link externo no diálogo Sobre: ") + url.toString());
    }
}

void SobreDialog::open_license_document(const QString& doc_id) {
    const QString content = license_document_contents_.value(doc_id);
    if (content.isEmpty()) {
        LogManager::error(QStringLiteral("Conteúdo da licença não disponível para '%1'").arg(doc_id));
        return;
    }

    const QString title = license_document_titles_.value(doc_id, doc_id);
    auto* dialog = new QDialog(nullptr);
    dialog->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                           Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    dialog->setModal(false);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    GerenciadorDialogos::instance().registrar_dialogo(dialog);
    dialog->setWindowTitle(title);
    dialog->resize(960, 720);
    dialog->setMinimumSize(640, 420);

    auto* layout = new QVBoxLayout(dialog);
    auto* editor = new QPlainTextEdit(dialog);
    editor->setReadOnly(true);
    editor->setPlainText(content);
    layout->addWidget(editor);

    const bool isDark = ThemeManager::instance().isDark();
    const QPalette pal = isDark ? ThemeManager::instance().createDarkPalette() : ThemeManager::instance().createLightPalette();
    dialog->setPalette(pal);
    editor->setPalette(pal);
    Win32DwmHelper::applyTitleBarTheme(dialog, isDark);

    dialog->show();
    GerenciadorDialogos::instance().trazer_para_frente(dialog);
}

int SobreDialog::calculate_fixed_label_height(int available_width) const {
    if (fixed_label_ == nullptr) return 0;
    const int margin = fixed_label_->margin();
    const QMargins contents_margins = fixed_label_->contentsMargins();
    const int text_width = available_width - contents_margins.left() - contents_margins.right() - (margin * 2);
    if (text_width <= 0) return fixed_label_->sizeHint().height();

    QTextDocument document;
    document.setDefaultFont(fixed_label_->font());
    document.setDocumentMargin(0);
    document.setHtml(fixed_label_->text());
    document.setTextWidth(text_width);

    return qCeil(document.size().height()) + contents_margins.top() + contents_margins.bottom() + (margin * 2);
}

void SobreDialog::refresh_fixed_label_geometry() {
    if (fixed_label_ == nullptr) return;
    if (layout() != nullptr) layout()->activate();
    const int available_width = width() - layout()->contentsMargins().left() - layout()->contentsMargins().right();
    const int target_height = calculate_fixed_label_height(available_width);
    fixed_label_->setFixedHeight(target_height);
}

void SobreDialog::update_tab_labels(int current_index) {
    if (tabs_ == nullptr) return;
    for (int i = 0; i < tab_labels_.size() && i < tabs_->count(); ++i) {
        tabs_->setTabText(i, (i == current_index) ? tab_labels_[i].second : tab_labels_[i].first);
    }
}

void SobreDialog::update_search_texts() {
    const bool pt = !current_lang_.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);
    if (search_label_) search_label_->setText(pt ? QStringLiteral("Buscar no conteúdo:") : QStringLiteral("Search content:"));
    if (search_edit_) search_edit_->setPlaceholderText(pt ? QStringLiteral("Digite um termo para localizar nas abas...") : QStringLiteral("Type a term to locate in tabs..."));
    if (search_previous_button_) search_previous_button_->setToolTip(pt ? QStringLiteral("Ocorrência anterior") : QStringLiteral("Previous occurrence"));
    if (search_next_button_) search_next_button_->setToolTip(pt ? QStringLiteral("Próxima ocorrência") : QStringLiteral("Next occurrence"));
}

void SobreDialog::reset_about_search_position() {
    search_current_browser_ = nullptr;
    search_current_start_ = -1;
}

void SobreDialog::apply_about_search() {
    if (search_edit_ == nullptr || tabs_ == nullptr) return;
    const QString query = search_edit_->text().trimmed();
    apply_about_search_highlights(tabs_->currentWidget(), query, search_current_browser_, search_current_start_);
}

void SobreDialog::navigate_about_search(int step) {
    if (search_edit_ == nullptr || tabs_ == nullptr) return;
    const QString query = search_edit_->text().trimmed();
    if (query.isEmpty()) return;

    QWidget* current_tab = tabs_->currentWidget();
    const QVector<AboutSearchMatch> matches = collect_about_search_matches(current_tab, query, true);
    if (matches.isEmpty()) {
        reset_about_search_position();
        update_about_search_buttons();
        return;
    }

    int current_index = find_about_search_match_index(matches, search_current_browser_, search_current_start_);
    if (current_index < 0) {
        current_index = (step >= 0) ? 0 : (matches.size() - 1);
    } else {
        current_index = (current_index + step + matches.size()) % matches.size();
    }

    const AboutSearchMatch& match = matches.at(current_index);
    search_current_browser_ = match.browser;
    search_current_start_ = match.start;

    apply_about_search_highlights(current_tab, query, search_current_browser_, search_current_start_);
    scroll_about_search_match_to_view(match);
    update_about_search_buttons();
}

void SobreDialog::update_about_search_buttons() {
    if (search_previous_button_ == nullptr || search_next_button_ == nullptr || tabs_ == nullptr) return;
    const QString query = search_edit_ ? search_edit_->text().trimmed() : QString();
    const bool has_matches = !query.isEmpty() && !collect_about_search_matches(tabs_->currentWidget(), query, true).isEmpty();
    search_previous_button_->setEnabled(has_matches);
    search_next_button_->setEnabled(has_matches);
}

bool SobreDialog::eventFilter(QObject* watched, QEvent* event) {
    if (watched == search_edit_ && event && event->type() == QEvent::KeyPress) {
        auto* key_event = static_cast<QKeyEvent*>(event);
        if (key_event->key() == Qt::Key_Up) {
            navigate_about_search(-1);
            return true;
        }
        if (key_event->key() == Qt::Key_Down) {
            navigate_about_search(1);
            return true;
        }
    }
    return QDialog::eventFilter(watched, event);
}

void SobreDialog::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);
    refresh_fixed_label_geometry();
}

void SobreDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    refresh_fixed_label_geometry();
    Win32DwmHelper::applyTitleBarTheme(this, ThemeManager::instance().isDark());
}

void SobreDialog::changeEvent(QEvent* event) {
    if (event != nullptr) {
        if (event->type() == QEvent::LanguageChange) {
            retranslateUi();
        } else if (event->type() == QEvent::ThemeChange || event->type() == QEvent::PaletteChange) {
            applyTheme(ThemeManager::instance().isDark());
        }
    }
    QDialog::changeEvent(event);
}

bool SobreDialog::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
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

void SobreDialog::on_idioma_alterado(const QString& novo_idioma) {
    retranslateUi(novo_idioma);
}

void SobreDialog::retranslateUi(const QString& novo_idioma) {
    if (!novo_idioma.isEmpty()) {
        current_lang_ = novo_idioma;
    } else {
        current_lang_ = QLocale().name();
    }
    const bool pt = !current_lang_.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);

    OpcoesSobre::carregar_textos_sobre();

    setWindowTitle(pt ? QStringLiteral("Sobre - Munin App - Leitor de Logs Massivos")
                      : QStringLiteral("About - Munin App - Massive Log Viewer"));

    if (fixed_label_ != nullptr) {
        fixed_label_->clear();
        const QString textColor = palette().color(QPalette::WindowText).name();
        fixed_label_->setText(pt ? QStringLiteral("<div style=\"margin:0; padding:0; color:%1;\"><span style=\"font-size:12pt; font-weight:600;\">MUNIN APP — LEITOR DE LOGS MASSIVOS</span><br><br><b>Versão:</b> 2026.11.6.0 (Win32 / Qt6)<br><b>Descrição:</b> Visualizador de arquivos de log gigantes com paginação instantânea e busca paralela.</div>").arg(textColor)
                                 : QStringLiteral("<div style=\"margin:0; padding:0; color:%1;\"><span style=\"font-size:12pt; font-weight:600;\">MUNIN APP — MASSIVE LOG VIEWER</span><br><br><b>Version:</b> 2026.11.6.0 (Win32 / Qt6)<br><b>Description:</b> High-performance viewer for massive log files with instant paging and parallel search.</div>").arg(textColor));
        refresh_fixed_label_geometry();
    }

    if (ok_button_ != nullptr) {
        ok_button_->setText(QStringLiteral("OK"));
    }

    info_not_available_text_ = pt ? QStringLiteral("Informação não disponível") : QStringLiteral("Information not available");
    site_oficial_text_ = pt ? QStringLiteral("Site Oficial") : QStringLiteral("Official Website");

    reload_browser_contents(palette());

    tab_labels_.clear();
    tab_labels_.push_back({pt ? QStringLiteral("Sobre") : QStringLiteral("About"),
                           pt ? QStringLiteral("Ocultar Sobre") : QStringLiteral("Hide About")});
    tab_labels_.push_back({pt ? QStringLiteral("Histórico") : QStringLiteral("History"),
                           pt ? QStringLiteral("Ocultar Histórico") : QStringLiteral("Hide History")});
    tab_labels_.push_back({pt ? QStringLiteral("EULA") : QStringLiteral("EULA"),
                           pt ? QStringLiteral("Ocultar EULA") : QStringLiteral("Hide EULA")});
    tab_labels_.push_back({pt ? QStringLiteral("Contrato Comercial") : QStringLiteral("Commercial Agreement"),
                           pt ? QStringLiteral("Ocultar Contrato") : QStringLiteral("Hide Agreement")});
    tab_labels_.push_back({pt ? QStringLiteral("Copyright e Marcas") : QStringLiteral("Copyright & Trademarks"),
                           pt ? QStringLiteral("Ocultar Copyright") : QStringLiteral("Hide Copyright")});
    tab_labels_.push_back({pt ? QStringLiteral("Licenças") : QStringLiteral("Licenses"),
                           pt ? QStringLiteral("Ocultar Licenças") : QStringLiteral("Hide Licenses")});
    tab_labels_.push_back({pt ? QStringLiteral("Avisos") : QStringLiteral("Notices"),
                           pt ? QStringLiteral("Ocultar Avisos") : QStringLiteral("Hide Notices")});
    tab_labels_.push_back({pt ? QStringLiteral("Privacidade") : QStringLiteral("Privacy"),
                           pt ? QStringLiteral("Ocultar Privacidade") : QStringLiteral("Hide Privacy")});
    tab_labels_.push_back({pt ? QStringLiteral("Notas de Lançamento") : QStringLiteral("Release Notes"),
                           pt ? QStringLiteral("Ocultar Notas") : QStringLiteral("Hide Notes")});

    if (tabs_ != nullptr) {
        update_tab_labels(tabs_->currentIndex());
    }

    update_search_texts();
}

void SobreDialog::reload_browser_contents(const QPalette& pal) {
    const bool pt = !current_lang_.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);
    const QPalette actualPal = pal.color(QPalette::Base).isValid() ? pal : palette();

    auto reload_b = [&](QTextBrowser* b, const std::function<void()>& setter) {
        if (!b) return;
        const int v = b->verticalScrollBar() ? b->verticalScrollBar()->value() : 0;
        const int h = b->horizontalScrollBar() ? b->horizontalScrollBar()->value() : 0;
        setter();
        if (b->document()) {
            b->document()->setDefaultStyleSheet(
                QStringLiteral("body, p, div, span, h1, h2, h3, h4, li { color: %1; } a { color: %2; }")
                    .arg(actualPal.color(QPalette::Text).name(), actualPal.color(QPalette::Link).name()));
        }
        if (b->verticalScrollBar()) b->verticalScrollBar()->setValue(v);
        if (b->horizontalScrollBar()) b->horizontalScrollBar()->setValue(h);
    };

    reload_b(about_browser_, [&]() {
        apply_text_or_placeholder(about_browser_, pt ? OpcoesSobre::ABOUT_TEXT_PT_BR : OpcoesSobre::ABOUT_TEXT_EN_US, info_not_available_text_, actualPal);
    });
    reload_b(history_browser_, [&]() {
        apply_text_or_placeholder(history_browser_, pt ? OpcoesSobre::History_APP_pt_BR : OpcoesSobre::History_APP_en_US, info_not_available_text_, actualPal);
    });
    reload_b(eula_browser_, [&]() {
        apply_text_or_placeholder(eula_browser_, pt ? OpcoesSobre::LICENSE_TEXT_PT_BR : OpcoesSobre::LICENSE_TEXT_EN_US, info_not_available_text_, actualPal);
    });
    reload_b(clc_browser_, [&]() {
        apply_text_or_placeholder(clc_browser_, pt ? OpcoesSobre::CLC_TEXT_PT_BR : OpcoesSobre::CLC_TEXT_EN_US, info_not_available_text_, actualPal);
    });
    reload_b(copyright_browser_, [&]() {
        apply_text_or_placeholder(copyright_browser_, pt ? OpcoesSobre::COPYRIGHT_TEXT_PT_BR : OpcoesSobre::COPYRIGHT_TEXT_EN_US, info_not_available_text_, actualPal);
    });
    reload_b(licencas_browser_, [&]() {
        licencas_browser_->setHtml(montar_html_licencas(
            OpcoesSobre::LICENSE_OWNER,
            site_oficial_text_,
            sites_licencas_,
            info_not_available_text_,
            displayed_path_to_doc_id_,
            actualPal));
    });
    reload_b(avisos_browser_, [&]() {
        apply_text_or_placeholder(avisos_browser_, pt ? OpcoesSobre::NOTICE_TEXT_PT_BR : OpcoesSobre::NOTICE_TEXT_EN_US, info_not_available_text_, actualPal);
    });
    reload_b(privacidade_browser_, [&]() {
        apply_text_or_placeholder(privacidade_browser_, pt ? OpcoesSobre::Privacy_Policy_pt_BR : OpcoesSobre::Privacy_Policy_en_US, info_not_available_text_, actualPal);
    });
    reload_b(release_notes_browser_, [&]() {
        apply_text_or_placeholder(release_notes_browser_, pt ? OpcoesSobre::RELEASE_NOTES_pt_BR : OpcoesSobre::RELEASE_NOTES_en_US, info_not_available_text_, actualPal);
    });
}

void SobreDialog::applyTheme(bool isDark) {
    Win32DwmHelper::applyTitleBarTheme(this, isDark);

    const QPalette pal = isDark ? ThemeManager::instance().createDarkPalette()
                                : ThemeManager::instance().createLightPalette();
    setPalette(pal);
    setAutoFillBackground(true);

    if (fixed_label_) {
        fixed_label_->setPalette(pal);
        fixed_label_->clear();
        const bool pt = !current_lang_.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);
        const QString textColor = pal.color(QPalette::WindowText).name();
        fixed_label_->setText(pt ? QStringLiteral("<div style=\"margin:0; padding:0; color:%1;\"><span style=\"font-size:12pt; font-weight:600;\">MUNIN APP — LEITOR DE LOGS MASSIVOS</span><br><br><b>Versão:</b> 2026.11.6.0 (Win32 / Qt6)<br><b>Descrição:</b> Visualizador de arquivos de log gigantes com paginação instantânea e busca paralela.</div>").arg(textColor)
                                 : QStringLiteral("<div style=\"margin:0; padding:0; color:%1;\"><span style=\"font-size:12pt; font-weight:600;\">MUNIN APP — MASSIVE LOG VIEWER</span><br><br><b>Version:</b> 2026.11.6.0 (Win32 / Qt6)<br><b>Description:</b> High-performance viewer for massive log files with instant paging and parallel search.</div>").arg(textColor));
        refresh_fixed_label_geometry();
    }
    if (search_label_) {
        search_label_->setPalette(pal);
    }
    if (search_edit_) {
        search_edit_->setPalette(pal);
    }
    if (search_previous_button_) {
        search_previous_button_->setPalette(pal);
    }
    if (search_next_button_) {
        search_next_button_->setPalette(pal);
    }
    if (ok_button_) {
        ok_button_->setPalette(pal);
    }
    if (tabs_) {
        tabs_->setPalette(pal);
        tabs_->setAutoFillBackground(true);
        if (tabs_->tabBar()) {
            tabs_->tabBar()->setPalette(pal);
            tabs_->tabBar()->setAutoFillBackground(true);
            tabs_->tabBar()->update();
        }
    }

    auto apply_browser_theme = [&](QTextBrowser* browser) {
        if (!browser) return;
        browser->setPalette(pal);
        browser->setAutoFillBackground(true);
        if (browser->viewport()) {
            browser->viewport()->setAutoFillBackground(true);
            browser->viewport()->setPalette(pal);
        }
        if (browser->verticalScrollBar()) {
            browser->verticalScrollBar()->setPalette(pal);
        }
        if (browser->horizontalScrollBar()) {
            browser->horizontalScrollBar()->setPalette(pal);
        }
    };

    apply_browser_theme(history_browser_);
    apply_browser_theme(detalhes_browser_);
    apply_browser_theme(licencas_browser_);
    apply_browser_theme(avisos_browser_);
    apply_browser_theme(privacidade_browser_);
    apply_browser_theme(release_notes_browser_);

    for (QTextBrowser* b : findChildren<QTextBrowser*>()) {
        apply_browser_theme(b);
    }

    reload_browser_contents(pal);

    apply_about_search();
    update();
    repaint();
}

void exibir_sobre(QWidget* parent, const QString& idioma) {
    GerenciadorDialogos::instance().exibir_sobre(parent, idioma);
}

} // namespace Munin::UI
