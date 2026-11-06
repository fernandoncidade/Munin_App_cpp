#pragma once

#include <QColor>
#include <QPalette>
#include <QPushButton>
#include <QTabWidget>
#include <QToolButton>
#include <QWidget>

namespace Munin::UI {

/**
 * @brief Foca o botão utilizando a política padrão nativa do Qt6.
 */
void focus_license_action_button(QPushButton *button);

/**
 * @brief Define o botão como default na janela utilizando a política padrão nativa do Qt6.
 */
void select_license_action_button(QPushButton *button);

/**
 * @brief Configura um QPushButton no padrão nativo do Qt6 (sem hacks de foco ou temas artificiais).
 */
void configure_license_action_button(QPushButton *button, int minWidth = 72);

/**
 * @brief Configura um QToolButton para navegação nativa com autoRaise e acessibilidade.
 */
void configure_tool_button(QToolButton *button, Qt::ArrowType arrowType, const QString &tooltip);

/**
 * @brief Configura tabs de documentos para renderização limpa no estilo nativo.
 */
void configure_document_tabs(QTabWidget *document_tabs);

/**
 * @brief Aplica cor de fundo Window a um widget através da QPalette nativa.
 */
void apply_window_bg(QWidget *widget, const QColor &color);

/**
 * @brief Aplica cor de fundo Base/AlternateBase a um container de listas ou visualização.
 */
void apply_list_bg(QWidget *widget, const QColor &color);

/**
 * @brief Aplica cor de fundo a campos de entrada via QPalette nativa.
 */
void apply_input_bg(QWidget *widget, const QColor &color);

/**
 * @brief Aplica a cor Base da paleta como fundo no widget de documento.
 */
void apply_document_base_background(QWidget *widget, const QPalette &sourcePalette);

/**
 * @brief Atualiza widgets filhos respeitando a paleta nativa.
 */
void refresh_document_base_backgrounds(QWidget *root, const QPalette &sourcePalette);

} // namespace Munin::UI
