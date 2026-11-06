#pragma once

#include <QHash>
#include <QMap>
#include <QObject>
#include <QString>

class QTranslator;

namespace source::language {

class GerenciadorTraducao : public QObject {
    Q_OBJECT

public:
    explicit GerenciadorTraducao(QObject *parent = nullptr);
    ~GerenciadorTraducao() override;
    bool aplicar_traducao();
    bool definir_idioma(const QString &codigo_idioma);
    QString obter_idioma_atual() const;
    void traduzir_botoes_padrao(QObject *dialogo) const;
    const QMap<QString, QString> &idiomas_disponiveis() const;
    QTranslator *obter_tradutor() const { return tradutor_; }

signals:
    void idioma_alterado(const QString &codigo_idioma);

private:
    void carregar_configuracao_idioma();
    void salvar_configuracao_idioma() const;
    QString obter_caminho_configuracao() const;
    void remover_tradutor_instalado();
    bool load_qt_translator_for_locale(const QString &codigo_idioma);

    QTranslator *tradutor_;
    QTranslator *qt_translator_;
    QHash<QString, QTranslator *> app_translators_cache_;
    QHash<QString, QTranslator *> qt_translators_cache_;
    QString idioma_atual_;
    QObject *app_;
    QString idioma_padrao_;
    QMap<QString, QString> idiomas_disponiveis_;
    QString dir_traducoes_;
};

}  // namespace source::language
