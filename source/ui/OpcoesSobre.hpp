#pragma once

#include <QString>
#include <QVector>

namespace Munin::UI {
struct DocumentoSobre;
}

namespace OpcoesSobre {
    extern const QString SITE_LICENSES;
    extern QString LICENSE_OWNER;
    extern QString LICENSE_TEXT_PT_BR;
    extern QString LICENSE_TEXT_EN_US;
    extern QString NOTICE_TEXT_PT_BR;
    extern QString NOTICE_TEXT_EN_US;
    extern QString ABOUT_TEXT_PT_BR;
    extern QString ABOUT_TEXT_EN_US;
    extern QString Privacy_Policy_pt_BR;
    extern QString Privacy_Policy_en_US;
    extern QString History_APP_pt_BR;
    extern QString History_APP_en_US;
    extern QString RELEASE_NOTES_pt_BR;
    extern QString RELEASE_NOTES_en_US;
    extern QString CLC_TEXT_PT_BR;
    extern QString CLC_TEXT_EN_US;
    extern QString COPYRIGHT_TEXT_PT_BR;
    extern QString COPYRIGHT_TEXT_EN_US;

    void carregar_textos_sobre();

    QVector<Munin::UI::DocumentoSobre> detalhes_documents(const QString& idioma);
    QVector<Munin::UI::DocumentoSobre> license_documents(const QString& idioma);
    QVector<Munin::UI::DocumentoSobre> notices_documents(const QString& idioma);
    QVector<Munin::UI::DocumentoSobre> privacy_policy_documents(const QString& idioma);
    QVector<Munin::UI::DocumentoSobre> release_notes_documents(const QString& idioma);
}
