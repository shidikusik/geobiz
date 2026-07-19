// SPDX-License-Identifier: MIT
#include "geobiz/app/AppConfig.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

#include <cstdlib>

namespace geobiz::app {

QString AppConfig::configFilePath() {
    const QString dir =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    return dir + "/geobiz.ini";
}

AppConfig AppConfig::load() {
    AppConfig cfg;

    // 1) Environment variable wins — convenient for CI and power users.
    if (const char* env = std::getenv("GEOBIZ_GOOGLE_MAPS_API_KEY");
        env != nullptr && env[0] != '\0') {
        cfg.googleMapsApiKey_ = QString::fromUtf8(env);
    }

    // 2) Fall back to the persisted config file.
    QSettings settings(configFilePath(), QSettings::IniFormat);
    if (cfg.googleMapsApiKey_.isEmpty()) {
        cfg.googleMapsApiKey_ =
            settings.value("google_maps_api_key").toString().trimmed();
    }

    cfg.datasetPath_ = settings.value("dataset_path").toString().trimmed();
    if (!cfg.datasetPath_.isEmpty() && !QFileInfo::exists(cfg.datasetPath_)) {
        cfg.datasetPath_.clear();  // ignore stale/broken path
    }

    return cfg;
}

bool AppConfig::saveGoogleMapsApiKey(const QString& key) {
    QSettings settings(configFilePath(), QSettings::IniFormat);
    settings.setValue("google_maps_api_key", key.trimmed());
    settings.sync();
    if (settings.status() == QSettings::NoError) {
        googleMapsApiKey_ = key.trimmed();
        return true;
    }
    return false;
}

}  // namespace geobiz::app
