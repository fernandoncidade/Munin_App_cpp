#include "GerenciadorTraducao.hpp"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QTranslator>
#include "source/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/LogManager.hpp"

namespace source::language {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

QString translations_dir()
{
    const QString app_dir = QCoreApplication::applicationDirPath();
    const QString current_dir = QDir::currentPath();
    const QStringList candidates = {
        QDir(app_dir).filePath(QStringLiteral("translations")),
        QDir(app_dir).filePath(QStringLiteral("language/translations")),
        QDir(app_dir).filePath(QStringLiteral("source/language/translations")),
        QDir(current_dir).filePath(QStringLiteral("source/language/translations")),
        QDir(current_dir).filePath(QStringLiteral("translations")),
        QDir(app_dir).filePath(QStringLiteral("../source/language/translations")),
    };

    for (const QString &candidate : candidates) {
        if (QDir(candidate).exists()) {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }

    return QFileInfo(candidates.constFirst()).absoluteFilePath();
}

QStringList translation_file_candidates(const QString &language_code)
{
    return {
        QStringLiteral("munin_%1.qm").arg(language_code),
    };
}

QStringList translation_path_candidates(const QString &translations_directory,
                                        const QString &language_code)
{
    QStringList paths;
    const QStringList files = translation_file_candidates(language_code);
    paths.reserve(files.size());
    for (const QString &file : files) {
        paths.append(QDir(translations_directory).filePath(file));
    }
    return paths;
}

}  // namespace

GerenciadorTraducao::GerenciadorTraducao(QObject *parent)
    : QObject(parent),
      tradutor_(nullptr),
      qt_translator_(nullptr),
      idioma_atual_(QStringLiteral("pt_BR")),
      app_(QCoreApplication::instance()),
      idioma_padrao_(QStringLiteral("pt_BR")),
      dir_traducoes_(translations_dir())
{
    idiomas_disponiveis_.insert(QStringLiteral("pt_BR"), QStringLiteral("Português (Brasil)"));
    idiomas_disponiveis_.insert(QStringLiteral("en_US"), QStringLiteral("English (United States)"));

    QDir().mkpath(dir_traducoes_);

    carregar_configuracao_idioma();
    aplicar_traducao();
}

GerenciadorTraducao::~GerenciadorTraducao()
{
    remover_tradutor_instalado();
    qDeleteAll(app_translators_cache_);
    app_translators_cache_.clear();
    qDeleteAll(qt_translators_cache_);
    qt_translators_cache_.clear();
}

bool GerenciadorTraducao::aplicar_traducao()
{
    auto *app = qobject_cast<QCoreApplication *>(app_ ? app_ : QCoreApplication::instance());
    if (app == nullptr) {
        return false;
    }

    remover_tradutor_instalado();
    QGuiApplication::setApplicationDisplayName(QString());

    const QString locale_code = QString(idioma_atual_).replace(QLatin1Char('_'), QLatin1Char('-'));
    QLocale::setDefault(QLocale(locale_code));

    // Cache lookup for Qt base translator
    if (qt_translators_cache_.contains(idioma_atual_)) {
        qt_translator_ = qt_translators_cache_.value(idioma_atual_);
        if (qt_translator_ != nullptr) {
            app->installTranslator(qt_translator_);
        }
    } else {
        load_qt_translator_for_locale(idioma_atual_);
        qt_translators_cache_.insert(idioma_atual_, qt_translator_);
    }

    // Cache lookup for application translator
    if (app_translators_cache_.contains(idioma_atual_)) {
        tradutor_ = app_translators_cache_.value(idioma_atual_);
        if (tradutor_ != nullptr) {
            app->installTranslator(tradutor_);
            QCoreApplication::postEvent(app, new QEvent(QEvent::LanguageChange));
            return true;
        }
        if (idioma_atual_ == idioma_padrao_) {
            QCoreApplication::postEvent(app, new QEvent(QEvent::LanguageChange));
            return true;
        }
    }

    bool aplicou_app_trad = false;
    for (const QString &translation_file : translation_file_candidates(idioma_atual_)) {
        const QString translation_path = QDir(dir_traducoes_).filePath(translation_file);
        if (!QFileInfo::exists(translation_path)) {
            continue;
        }

        auto *new_trad = new QTranslator(this);
        if (new_trad->load(translation_path)) {
            tradutor_ = new_trad;
            app->installTranslator(tradutor_);
            app_translators_cache_.insert(idioma_atual_, tradutor_);
            aplicou_app_trad = true;
            break;
        }

        logger()->error(QStringLiteral("Erro ao carregar arquivo de tradução: %1").arg(translation_path));
        delete new_trad;
    }

    if (!aplicou_app_trad && !app_translators_cache_.contains(idioma_atual_)) {
        app_translators_cache_.insert(idioma_atual_, nullptr);
    }

    QCoreApplication::postEvent(app, new QEvent(QEvent::LanguageChange));

    if (aplicou_app_trad || qt_translator_ != nullptr || idioma_atual_ == idioma_padrao_) {
        return true;
    }

    const QStringList candidates = translation_path_candidates(dir_traducoes_, idioma_atual_);
    logger()->warning(QStringLiteral("Nenhum arquivo de tradução encontrado para %1. Candidatos: %2")
                          .arg(idioma_atual_, candidates.join(QStringLiteral(", "))));
    return false;
}

bool GerenciadorTraducao::definir_idioma(const QString &codigo_idioma)
{
    if (!idiomas_disponiveis_.contains(codigo_idioma)) {
        return false;
    }

    idioma_atual_ = codigo_idioma;
    salvar_configuracao_idioma();
    const bool resultado = aplicar_traducao();
    emit idioma_alterado(codigo_idioma);
    return resultado;
}

QString GerenciadorTraducao::obter_idioma_atual() const
{
    return idioma_atual_;
}

void GerenciadorTraducao::traduzir_botoes_padrao(QObject *dialogo) const
{
    if (auto *message_box = qobject_cast<QMessageBox *>(dialogo)) {
        const struct ButtonText {
            QMessageBox::StandardButton button;
            QString text;
        } buttons[] = {
            {QMessageBox::Ok, QStringLiteral("OK")},
            {QMessageBox::Cancel, QStringLiteral("Cancelar")},
            {QMessageBox::Yes, QStringLiteral("Sim")},
            {QMessageBox::No, QStringLiteral("Não")},
            {QMessageBox::Abort, QStringLiteral("Abortar")},
            {QMessageBox::Retry, QStringLiteral("Tentar Novamente")},
            {QMessageBox::Ignore, QStringLiteral("Ignorar")},
            {QMessageBox::Close, QStringLiteral("Fechar")},
            {QMessageBox::Help, QStringLiteral("Ajuda")},
            {QMessageBox::Apply, QStringLiteral("Aplicar")},
            {QMessageBox::Reset, QStringLiteral("Redefinir")},
            {QMessageBox::RestoreDefaults, QStringLiteral("Restaurar Padrões")},
            {QMessageBox::Save, QStringLiteral("Salvar")},
            {QMessageBox::SaveAll, QStringLiteral("Salvar Tudo")},
            {QMessageBox::Open, QStringLiteral("Abrir")},
        };

        for (const ButtonText &button : buttons) {
            if (QAbstractButton *widget = message_box->button(button.button)) {
                widget->setText(QCoreApplication::translate("InterfaceGrafica", button.text.toUtf8().constData()));
            }
        }
    } else if (auto *button_box = qobject_cast<QDialogButtonBox *>(dialogo)) {
        const struct ButtonText {
            QDialogButtonBox::StandardButton button;
            QString text;
        } buttons[] = {
            {QDialogButtonBox::Ok, QStringLiteral("OK")},
            {QDialogButtonBox::Cancel, QStringLiteral("Cancelar")},
            {QDialogButtonBox::Yes, QStringLiteral("Sim")},
            {QDialogButtonBox::No, QStringLiteral("Não")},
            {QDialogButtonBox::Abort, QStringLiteral("Abortar")},
            {QDialogButtonBox::Retry, QStringLiteral("Tentar Novamente")},
            {QDialogButtonBox::Ignore, QStringLiteral("Ignorar")},
            {QDialogButtonBox::Close, QStringLiteral("Fechar")},
            {QDialogButtonBox::Help, QStringLiteral("Ajuda")},
            {QDialogButtonBox::Apply, QStringLiteral("Aplicar")},
            {QDialogButtonBox::Reset, QStringLiteral("Redefinir")},
            {QDialogButtonBox::RestoreDefaults, QStringLiteral("Restaurar Padrões")},
            {QDialogButtonBox::Save, QStringLiteral("Salvar")},
            {QDialogButtonBox::SaveAll, QStringLiteral("Salvar Tudo")},
            {QDialogButtonBox::Open, QStringLiteral("Abrir")},
        };

        for (const ButtonText &button : buttons) {
            if (QAbstractButton *widget = button_box->button(button.button)) {
                widget->setText(QCoreApplication::translate("InterfaceGrafica", button.text.toUtf8().constData()));
            }
        }
    }
}

const QMap<QString, QString> &GerenciadorTraducao::idiomas_disponiveis() const
{
    return idiomas_disponiveis_;
}

void GerenciadorTraducao::carregar_configuracao_idioma()
{
    QFile file(obter_caminho_configuracao());
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonObject config = doc.object();
    if (config.contains(QStringLiteral("idioma"))) {
        idioma_atual_ = config.value(QStringLiteral("idioma")).toString(idioma_atual_);
    }
}

void GerenciadorTraducao::salvar_configuracao_idioma() const
{
    const QString config_path = obter_caminho_configuracao();
    QDir().mkpath(QFileInfo(config_path).absolutePath());

    QSaveFile file(config_path);
    if (!file.open(QIODevice::WriteOnly)) {
        logger()->error(QStringLiteral("Erro ao salvar configuração de idioma: %1").arg(config_path));
        return;
    }

    QJsonObject config;
    config.insert(QStringLiteral("idioma"), idioma_atual_);
    const QByteArray payload = QJsonDocument(config).toJson(QJsonDocument::Indented);
    if (file.write(payload) != payload.size() || !file.commit()) {
        logger()->error(QStringLiteral("Erro ao confirmar configuração de idioma: %1").arg(config_path));
    }
}

QString GerenciadorTraducao::obter_caminho_configuracao() const
{
    return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("language.json"));
}

void GerenciadorTraducao::remover_tradutor_instalado()
{
    auto *app = qobject_cast<QCoreApplication *>(app_ ? app_ : QCoreApplication::instance());
    if (app == nullptr) {
        return;
    }

    if (tradutor_ != nullptr) {
        app->removeTranslator(tradutor_);
        tradutor_ = nullptr;
    }

    if (qt_translator_ != nullptr) {
        app->removeTranslator(qt_translator_);
        qt_translator_ = nullptr;
    }
}

bool GerenciadorTraducao::load_qt_translator_for_locale(const QString &codigo_idioma)
{
    auto *app = qobject_cast<QCoreApplication *>(app_ ? app_ : QCoreApplication::instance());
    if (app == nullptr) {
        return false;
    }

    const QString trans_dir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if (!QDir(trans_dir).exists()) {
        return false;
    }

    const QLocale locale(codigo_idioma);
    const QStringList preferred_catalogs = {
        QStringLiteral("qtbase"),
        QStringLiteral("qt"),
    };
    for (const QString &catalog : preferred_catalogs) {
        auto *translator = new QTranslator(this);
        if (translator->load(locale, catalog, QStringLiteral("_"), trans_dir)) {
            qt_translator_ = translator;
            app->installTranslator(qt_translator_);
            return true;
        }
        delete translator;
    }

    const QString alvo = codigo_idioma.toLower();
    const QString curto = alvo.section(QLatin1Char('_'), 0, 0);
    const QFileInfoList files =
        QDir(trans_dir).entryInfoList(QStringList() << QStringLiteral("*.qm"), QDir::Files);

    QString chosen;
    for (const QFileInfo &file : files) {
        const QString name = file.fileName().toLower();
        if (name == QStringLiteral("qtbase_%1.qm").arg(alvo)) {
            chosen = file.absoluteFilePath();
            break;
        }
        if (chosen.isEmpty() && name == QStringLiteral("qtbase_%1.qm").arg(curto)) {
            chosen = file.absoluteFilePath();
        }
    }

    if (chosen.isEmpty()) {
        return false;
    }

    qt_translator_ = new QTranslator(this);
    if (!qt_translator_->load(chosen)) {
        qt_translator_->deleteLater();
        qt_translator_ = nullptr;
        return false;
    }

    app->installTranslator(qt_translator_);
    return true;
}

}  // namespace source::language
