// SPDX-License-Identifier: MIT
//
// AppConfig.hpp — runtime configuration, primarily the Google API key.
//
// The Google Maps / Places / Geocoding key is never hard-coded. It is resolved,
// in priority order, from:
//   1. the GEOBIZ_GOOGLE_MAPS_API_KEY environment variable,
//   2. a `google_maps_api_key` entry in the platform config file
//      (QStandardPaths::AppConfigLocation/geobiz.ini),
//   3. an empty value — in which case the UI shows setup instructions instead of
//      silently loading a broken map.

#pragma once

#include <QString>

namespace geobiz::app {

class AppConfig {
public:
    /// Loads configuration from the environment and the per-user config file.
    static AppConfig load();

    [[nodiscard]] QString googleMapsApiKey() const { return googleMapsApiKey_; }
    [[nodiscard]] bool hasGoogleMapsApiKey() const {
        return !googleMapsApiKey_.isEmpty();
    }

    /// Absolute path to an external dataset file, if the user configured one to
    /// override the bundled resource (used to feed real open-data without a
    /// rebuild). Empty ⇒ use the bundled ":/data/uzbekistan.json".
    [[nodiscard]] QString datasetPath() const { return datasetPath_; }

    /// Persists the Google Maps API key to the per-user config file so the user
    /// only needs to enter it once. Returns true on success.
    bool saveGoogleMapsApiKey(const QString& key);

private:
    QString googleMapsApiKey_;
    QString datasetPath_;

    static QString configFilePath();
};

}  // namespace geobiz::app
