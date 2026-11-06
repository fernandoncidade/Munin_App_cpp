#include "GerenciadorDialogos.hpp"
#include "SobreDialog.hpp"
#include "Manual.hpp"
#include "LogGeneratorDialog.hpp"
#include "OpcoesSobre.hpp"
#include "ThemeManager.hpp"
#include "Win32DwmHelper.hpp"
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

#include <QCoreApplication>
#include <QEvent>
#include <QStyle>

namespace Munin::UI {

GerenciadorDialogos& GerenciadorDialogos::instance() {
    static GerenciadorDialogos s_instance;
    return s_instance;
}

GerenciadorDialogos::GerenciadorDialogos(QObject* parent)
    : QObject(parent) {
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &GerenciadorDialogos::atualizar_tema);
}

void GerenciadorDialogos::inicializar(QWidget* mainWindow) {
    if (m_mainWindow && m_mainWindow.data() != mainWindow) {
        m_mainWindow->removeEventFilter(this);
    }
    m_mainWindow = mainWindow;
    if (m_mainWindow) {
        m_mainWindow->installEventFilter(this);
        connect(m_mainWindow.data(), &QObject::destroyed, this, [this]() {
            fechar_todos();
        });
    }
}

void GerenciadorDialogos::registrar_dialogo(QDialog* dialog) {
    if (dialog == nullptr) return;

    // Configuração nativa de janela de nível superior estritamente independente (top-level modeless).
    // O diálogo não possui parent/owner para que o Windows DWM não o trave acima da GUI principal,
    // permitindo que o usuário clique ou ative a GUI principal trazendo-a para a frente livremente
    // e empurrando os diálogos para trás no eixo Z (Z-Order).
    if (dialog->parent() != nullptr) {
        dialog->setParent(nullptr);
    }
    dialog->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                           Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    dialog->setModal(false);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

#if defined(Q_OS_WIN)
    HWND hwnd = reinterpret_cast<HWND>(dialog->winId());
    if (hwnd) {
        ::SetWindowLongPtrW(hwnd, GWLP_HWNDPARENT, 0);
    }
#endif

    dialog->installEventFilter(this);

    m_dialogos.removeAll(dialog);
    m_dialogos.append(dialog);

    connect(dialog, &QObject::destroyed, this, [this, dialog]() {
        remover_dialogo(dialog);
    });
}

void GerenciadorDialogos::remover_dialogo(QDialog* dialog) {
    m_dialogos.removeAll(dialog);
}

void GerenciadorDialogos::trazer_para_frente(QDialog* dialog) {
    if (dialog == nullptr) return;

    m_dialogos.removeAll(dialog);
    m_dialogos.append(dialog);

    dialog->raise();
    dialog->activateWindow();
}

void GerenciadorDialogos::reordenar_hierarquia() {
    m_dialogos.removeAll(nullptr);
}

void GerenciadorDialogos::fechar_todos() {
    m_dialogos.removeAll(nullptr);
    auto lista = m_dialogos;
    m_dialogos.clear();
    for (auto& ptr : lista) {
        if (QDialog* d = ptr.data()) {
            d->close();
        }
    }
}

bool GerenciadorDialogos::eventFilter(QObject* watched, QEvent* event) {
    if (event != nullptr) {
        if (watched == m_mainWindow.data()) {
            if (event->type() == QEvent::Close) {
                fechar_todos();
            } else if (event->type() == QEvent::WindowActivate) {
                if (m_mainWindow) {
                    m_mainWindow->raise();
                }
            }
            return QObject::eventFilter(watched, event);
        }

        if (event->type() == QEvent::Show) {
            if (auto* dialog = qobject_cast<QDialog*>(watched)) {
                Win32DwmHelper::applyTitleBarTheme(dialog, ThemeManager::instance().isDark());
#if defined(Q_OS_WIN)
                HWND hwnd = reinterpret_cast<HWND>(dialog->winId());
                if (hwnd) {
                    ::SetWindowLongPtrW(hwnd, GWLP_HWNDPARENT, 0);
                }
#endif
            }
        } else if (event->type() == QEvent::WindowActivate) {
            if (auto* dialog = qobject_cast<QDialog*>(watched)) {
                if (m_dialogos.contains(dialog)) {
                    if (m_dialogos.last() != dialog) {
                        m_dialogos.removeAll(dialog);
                        m_dialogos.append(dialog);
                    }
                    dialog->raise();
                }
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

void GerenciadorDialogos::atualizar_traducoes(const QString& novo_idioma) {
    if (!novo_idioma.isEmpty()) {
        m_idiomaAtual = novo_idioma;
    }
    m_dialogos.removeAll(nullptr);

    for (int i = 0; i < m_dialogos.size(); ++i) {
        if (QDialog* d = m_dialogos[i].data()) {
            if (auto* sobre = qobject_cast<SobreDialog*>(d)) {
                sobre->retranslateUi(m_idiomaAtual);
            } else if (auto* manual = qobject_cast<ManualDialog*>(d)) {
                manual->atualizar_traducao(m_idiomaAtual);
            } else if (auto* gen = qobject_cast<LogGeneratorDialog*>(d)) {
                gen->retranslateUi();
            } else {
                QCoreApplication::postEvent(d, new QEvent(QEvent::LanguageChange));
            }
        }
    }
}

void GerenciadorDialogos::atualizar_tema(bool isDark) {
    m_dialogos.removeAll(nullptr);

    for (int i = 0; i < m_dialogos.size(); ++i) {
        if (QDialog* d = m_dialogos[i].data()) {
            Win32DwmHelper::applyTitleBarTheme(d, isDark);
            if (auto* sobre = qobject_cast<SobreDialog*>(d)) {
                sobre->applyTheme(isDark);
            } else if (auto* manual = qobject_cast<ManualDialog*>(d)) {
                manual->applyTheme(isDark);
            } else if (auto* gen = qobject_cast<LogGeneratorDialog*>(d)) {
                gen->applyTheme(isDark);
            } else {
                const QPalette pal = isDark ? ThemeManager::instance().createDarkPalette()
                                            : ThemeManager::instance().createLightPalette();
                d->setPalette(pal);
                for (auto* w : d->findChildren<QWidget*>()) {
                    if (w) {
                        w->setPalette(pal);
                        w->update();
                    }
                }
                d->update();
                d->repaint();
            }
        }
    }
}

SobreDialog* GerenciadorDialogos::obter_sobre_dialogo() const {
    for (const auto& ptr : m_dialogos) {
        if (auto* sobre = qobject_cast<SobreDialog*>(ptr.data())) {
            return sobre;
        }
    }
    return nullptr;
}

ManualDialog* GerenciadorDialogos::obter_manual_dialogo() const {
    for (const auto& ptr : m_dialogos) {
        if (auto* manual = qobject_cast<ManualDialog*>(ptr.data())) {
            return manual;
        }
    }
    return nullptr;
}

LogGeneratorDialog* GerenciadorDialogos::obter_gerador_dialogo() const {
    for (const auto& ptr : m_dialogos) {
        if (auto* gen = qobject_cast<LogGeneratorDialog*>(ptr.data())) {
            return gen;
        }
    }
    return nullptr;
}

void GerenciadorDialogos::exibir_sobre(QWidget* /*parent*/, const QString& idioma) {
    QString lang = idioma.isEmpty() ? m_idiomaAtual : idioma;
    if (lang.isEmpty()) lang = QStringLiteral("pt_BR");

    if (SobreDialog* existing = obter_sobre_dialogo()) {
        existing->retranslateUi(lang);
        existing->show();
        trazer_para_frente(existing);
        return;
    }

    OpcoesSobre::carregar_textos_sobre();
    const bool pt = !lang.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);

    QString title = pt ? QStringLiteral("Sobre - Munin App - Leitor de Logs Massivos")
                       : QStringLiteral("About - Munin App - Massive Log Viewer");
    QString fixed = pt ? QStringLiteral("<div style=\"margin:0; padding:0;\"><span style=\"font-size:12pt; font-weight:600;\">MUNIN APP — LEITOR DE LOGS MASSIVOS</span><br><br><b>Versão:</b> 2026.11.6.0 (Win32 / Qt6)<br><b>Descrição:</b> Visualizador de arquivos de log gigantes com paginação instantânea e busca paralela.</div>")
                       : QStringLiteral("<div style=\"margin:0; padding:0;\"><span style=\"font-size:12pt; font-weight:600;\">MUNIN APP — MASSIVE LOG VIEWER</span><br><br><b>Version:</b> 2026.11.6.0 (Win32 / Qt6)<br><b>Description:</b> High-performance viewer for massive log files with instant paging and parallel search.</div>");
    QString history = pt ? OpcoesSobre::History_APP_pt_BR : OpcoesSobre::History_APP_en_US;
    QString details = pt ? OpcoesSobre::ABOUT_TEXT_PT_BR : OpcoesSobre::ABOUT_TEXT_EN_US;
    QString licenses = pt ? OpcoesSobre::LICENSE_TEXT_PT_BR : OpcoesSobre::LICENSE_TEXT_EN_US;
    const QString clc = pt ? OpcoesSobre::CLC_TEXT_PT_BR : OpcoesSobre::CLC_TEXT_EN_US;
    const QString cpr = pt ? OpcoesSobre::COPYRIGHT_TEXT_PT_BR : OpcoesSobre::COPYRIGHT_TEXT_EN_US;
    QString siteLicenses = OpcoesSobre::SITE_LICENSES;
    QString releaseNotes = pt ? OpcoesSobre::RELEASE_NOTES_pt_BR : OpcoesSobre::RELEASE_NOTES_en_US;

    auto* dialog = new SobreDialog(
        nullptr,
        title,
        fixed,
        history,
        details,
        licenses,
        siteLicenses,
        pt ? QStringLiteral("Histórico") : QStringLiteral("History"),
        pt ? QStringLiteral("Ocultar Histórico") : QStringLiteral("Hide History"),
        pt ? QStringLiteral("Detalhes") : QStringLiteral("Details"),
        pt ? QStringLiteral("Ocultar Detalhes") : QStringLiteral("Hide Details"),
        pt ? QStringLiteral("Licença") : QStringLiteral("Licenses"),
        pt ? QStringLiteral("Ocultar Licença") : QStringLiteral("Hide Licenses"),
        QStringLiteral("OK"),
        pt ? QStringLiteral("Site Oficial") : QStringLiteral("Official Website"),
        pt ? OpcoesSobre::NOTICE_TEXT_PT_BR : OpcoesSobre::NOTICE_TEXT_EN_US,
        pt ? QStringLiteral("Avisos") : QStringLiteral("Notices"),
        pt ? QStringLiteral("Ocultar Avisos") : QStringLiteral("Hide Notices"),
        pt ? OpcoesSobre::Privacy_Policy_pt_BR : OpcoesSobre::Privacy_Policy_en_US,
        pt ? QStringLiteral("Privacidade") : QStringLiteral("Privacy"),
        pt ? QStringLiteral("Ocultar Privacidade") : QStringLiteral("Hide Privacy"),
        pt ? QStringLiteral("Informação não disponível") : QStringLiteral("Information not available"),
        releaseNotes,
        pt ? QStringLiteral("Notas de Lançamento") : QStringLiteral("Release Notes"),
        pt ? QStringLiteral("Ocultar Notas") : QStringLiteral("Hide Notes"),
        clc,
        cpr);

    registrar_dialogo(dialog);
    dialog->show();
    trazer_para_frente(dialog);
}

void GerenciadorDialogos::exibir_manual(QWidget* /*parent*/, const QString& idioma) {
    QString lang = idioma.isEmpty() ? m_idiomaAtual : idioma;
    if (lang.isEmpty()) lang = QStringLiteral("pt_BR");

    if (ManualDialog* existing = obter_manual_dialogo()) {
        existing->atualizar_traducao(lang);
        existing->show();
        trazer_para_frente(existing);
        return;
    }

    auto* dialog = new ManualDialog(nullptr, lang);
    registrar_dialogo(dialog);
    dialog->show();
    trazer_para_frente(dialog);
}

void GerenciadorDialogos::exibir_gerador(QWidget* /*parent*/) {
    if (LogGeneratorDialog* existing = obter_gerador_dialogo()) {
        existing->retranslateUi();
        existing->show();
        trazer_para_frente(existing);
        return;
    }

    auto* dialog = new LogGeneratorDialog(nullptr);
    registrar_dialogo(dialog);
    dialog->show();
    trazer_para_frente(dialog);
}

} // namespace Munin::UI
