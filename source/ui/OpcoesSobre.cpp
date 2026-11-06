#include "OpcoesSobre.hpp"
#include "SobreDialog.hpp"
#include "source/utils/ApplicationPathUtils.hpp"
#include "source/utils/LogManager.hpp"

namespace OpcoesSobre {

const QString SITE_LICENSES =
    "https://www.gnu.org/licenses/lgpl-3.0.html.en\n"
    "https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.en\n"
    "https://opensource.org/licenses/BSD-2-Clause\n"
    "https://opensource.org/licenses/BSD-3-Clause\n"
    "https://www.apache.org/licenses/LICENSE-2.0\n"
    "https://opensource.org/licenses/MIT\n"
    "https://opensource.org/license/isc-license-txt\n";

QString LICENSE_OWNER;
QString LICENSE_TEXT_PT_BR;
QString LICENSE_TEXT_EN_US;
QString NOTICE_TEXT_PT_BR;
QString NOTICE_TEXT_EN_US;
QString ABOUT_TEXT_PT_BR;
QString ABOUT_TEXT_EN_US;
QString Privacy_Policy_pt_BR;
QString Privacy_Policy_en_US;
QString History_APP_pt_BR;
QString History_APP_en_US;
QString RELEASE_NOTES_pt_BR;
QString RELEASE_NOTES_en_US;
QString CLC_TEXT_PT_BR;
QString CLC_TEXT_EN_US;
QString COPYRIGHT_TEXT_PT_BR;
QString COPYRIGHT_TEXT_EN_US;

static bool s_loaded = false;

void carregar_textos_sobre() {
    if (s_loaded) return;
    try {
        LICENSE_OWNER = load_text_file(QStringLiteral("License.txt"), QStringLiteral("LICENSES"));

        LICENSE_TEXT_PT_BR = load_text_file(QStringLiteral("EULA_pt_BR.txt"), QStringLiteral("EULA"));
        LICENSE_TEXT_EN_US = load_text_file(QStringLiteral("EULA_en_US.txt"), QStringLiteral("EULA"));

        NOTICE_TEXT_PT_BR = load_text_file(QStringLiteral("NOTICE_pt_BR.txt"), QStringLiteral("NOTICES"));
        NOTICE_TEXT_EN_US = load_text_file(QStringLiteral("NOTICE_en_US.txt"), QStringLiteral("NOTICES"));

        ABOUT_TEXT_PT_BR = load_text_file(QStringLiteral("ABOUT_pt_BR.txt"), QStringLiteral("ABOUT"));
        ABOUT_TEXT_EN_US = load_text_file(QStringLiteral("ABOUT_en_US.txt"), QStringLiteral("ABOUT"));

        Privacy_Policy_pt_BR = load_text_file(QStringLiteral("Privacy_Policy_pt_BR.txt"), QStringLiteral("PRIVACY_POLICY"));
        Privacy_Policy_en_US = load_text_file(QStringLiteral("Privacy_Policy_en_US.txt"), QStringLiteral("PRIVACY_POLICY"));

        History_APP_pt_BR = load_text_file(QStringLiteral("History_APP_pt_BR.txt"), QStringLiteral("ABOUT"));
        History_APP_en_US = load_text_file(QStringLiteral("History_APP_en_US.txt"), QStringLiteral("ABOUT"));

        RELEASE_NOTES_pt_BR = load_text_file(QStringLiteral("RELEASE NOTES_pt_BR.txt"), QStringLiteral("RELEASE"));
        RELEASE_NOTES_en_US = load_text_file(QStringLiteral("RELEASE NOTES_en_US.txt"), QStringLiteral("RELEASE"));

        CLC_TEXT_PT_BR = load_text_file(QStringLiteral("CLC_pt_BR.txt"), QStringLiteral("CLC"));
        CLC_TEXT_EN_US = load_text_file(QStringLiteral("CLC_en_US.txt"), QStringLiteral("CLC"));

        COPYRIGHT_TEXT_PT_BR = load_text_file(QStringLiteral("AVISO DE COPYRIGHT E MARCA REGISTRADA_pt_BR.txt"), QStringLiteral("COPYRIGHT"));
        if (COPYRIGHT_TEXT_PT_BR.isEmpty()) {
            COPYRIGHT_TEXT_PT_BR = load_text_file(QStringLiteral("COPYRIGHT_pt_BR.txt"), QStringLiteral("COPYRIGHT"));
        }
        COPYRIGHT_TEXT_EN_US = load_text_file(QStringLiteral("COPYRIGHT AND TRADEMARK NOTICE_en_US.txt"), QStringLiteral("COPYRIGHT"));
        if (COPYRIGHT_TEXT_EN_US.isEmpty()) {
            COPYRIGHT_TEXT_EN_US = load_text_file(QStringLiteral("COPYRIGHT_en_US.txt"), QStringLiteral("COPYRIGHT"));
        }

        s_loaded = true;
    } catch (const std::exception& e) {
        LogManager::error(QStringLiteral("Erro ao carregar textos de opções sobre: ") + QString::fromLatin1(e.what()));
    }
}

QVector<Munin::UI::DocumentoSobre> detalhes_documents(const QString& idioma) {
    carregar_textos_sobre();
    bool en = idioma.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);
    return {
        { en ? QStringLiteral("Munin App") : QStringLiteral("Munin App"),
          en ? ABOUT_TEXT_EN_US : ABOUT_TEXT_PT_BR }
    };
}

QVector<Munin::UI::DocumentoSobre> license_documents(const QString& idioma) {
    carregar_textos_sobre();
    bool en = idioma.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);
    QVector<Munin::UI::DocumentoSobre> docs;
    docs.append({ en ? QStringLiteral("EULA / License") : QStringLiteral("EULA / Licença"),
                  en ? LICENSE_TEXT_EN_US : LICENSE_TEXT_PT_BR });
    if (!CLC_TEXT_PT_BR.isEmpty() || !CLC_TEXT_EN_US.isEmpty()) {
        docs.append({ en ? QStringLiteral("Commercial Agreement (CLC)") : QStringLiteral("Contrato Comercial (CLC)"),
                      en ? CLC_TEXT_EN_US : CLC_TEXT_PT_BR });
    }
    if (!COPYRIGHT_TEXT_PT_BR.isEmpty() || !COPYRIGHT_TEXT_EN_US.isEmpty()) {
        docs.append({ en ? QStringLiteral("Copyright & Trademark") : QStringLiteral("Copyright e Marca Registrada"),
                      en ? COPYRIGHT_TEXT_EN_US : COPYRIGHT_TEXT_PT_BR });
    }
    if (!LICENSE_OWNER.isEmpty()) {
        docs.append({ QStringLiteral("License Owner"), LICENSE_OWNER });
    }
    return docs;
}

QVector<Munin::UI::DocumentoSobre> notices_documents(const QString& idioma) {
    carregar_textos_sobre();
    bool en = idioma.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);
    return {
        { en ? QStringLiteral("Notices") : QStringLiteral("Avisos de Terceiros"),
          en ? NOTICE_TEXT_EN_US : NOTICE_TEXT_PT_BR }
    };
}

QVector<Munin::UI::DocumentoSobre> privacy_policy_documents(const QString& idioma) {
    carregar_textos_sobre();
    bool en = idioma.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);
    return {
        { en ? QStringLiteral("Privacy Policy") : QStringLiteral("Política de Privacidade"),
          en ? Privacy_Policy_en_US : Privacy_Policy_pt_BR }
    };
}

QVector<Munin::UI::DocumentoSobre> release_notes_documents(const QString& idioma) {
    carregar_textos_sobre();
    bool en = idioma.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);
    return {
        { en ? QStringLiteral("Release Notes") : QStringLiteral("Notas de Lançamento"),
          en ? RELEASE_NOTES_en_US : RELEASE_NOTES_pt_BR }
    };
}

} // namespace OpcoesSobre
