#include "CaminhoPersistenteUtils.hpp"
#include "LogManager.hpp"
#include <QDir>
#include <QStandardPaths>

namespace source::utils {

QString obter_caminho_persistente() {
    QString localAppData = qEnvironmentVariable("LOCALAPPDATA");
    if (localAppData.isEmpty()) {
        localAppData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    }

    QDir configDir(QDir(localAppData).filePath(QStringLiteral("Munin App")));
    if (!configDir.exists()) {
        if (!configDir.mkpath(QStringLiteral("."))) {
            LogManager::error(QStringLiteral("Erro ao criar diretório de configuração persistente: ") + configDir.absolutePath());
        }
    }
    return configDir.absolutePath();
}

QString obterCaminhoPersistente() {
    return obter_caminho_persistente();
}

} // namespace source::utils

QString obter_caminho_persistente() {
    return source::utils::obter_caminho_persistente();
}

QString obterCaminhoPersistente() {
    return source::utils::obterCaminhoPersistente();
}
