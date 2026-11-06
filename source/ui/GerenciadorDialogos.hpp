#pragma once

#include <QDialog>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QWidget>

namespace Munin::UI {

class SobreDialog;
class ManualDialog;
class LogGeneratorDialog;

/**
 * @brief Gerenciador centralizado de janelas de diálogo do Munin.
 * Controla a hierarquização nativa (ordem Z do mais recente para o mais antigo,
 * mantendo o diálogo ativo na frente e a GUI principal na base) e orquestra
 * a tradução dinâmica e imediata de todas as janelas de diálogo abertas em tempo de execução.
 */
class GerenciadorDialogos : public QObject {
    Q_OBJECT

public:
    static GerenciadorDialogos& instance();

    void inicializar(QWidget* mainWindow);
    QWidget* mainWindow() const { return m_mainWindow.data(); }

    void registrar_dialogo(QDialog* dialog);
    void remover_dialogo(QDialog* dialog);
    void trazer_para_frente(QDialog* dialog);
    void reordenar_hierarquia();
    void fechar_todos();

    void atualizar_traducoes(const QString& novo_idioma);
    void atualizar_tema(bool isDark);
    QString obter_idioma_atual() const { return m_idiomaAtual; }

    SobreDialog* obter_sobre_dialogo() const;
    ManualDialog* obter_manual_dialogo() const;
    LogGeneratorDialog* obter_gerador_dialogo() const;

    void exibir_sobre(QWidget* parent = nullptr, const QString& idioma = QString());
    void exibir_manual(QWidget* parent = nullptr, const QString& idioma = QString());
    void exibir_gerador(QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    explicit GerenciadorDialogos(QObject* parent = nullptr);
    ~GerenciadorDialogos() override = default;

    QPointer<QWidget> m_mainWindow;
    QList<QPointer<QDialog>> m_dialogos;
    bool m_reordenando{false};
    QString m_idiomaAtual{QStringLiteral("pt_BR")};
};

} // namespace Munin::UI
