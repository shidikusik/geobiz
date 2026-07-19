// SPDX-License-Identifier: MIT
//
// LocalDatasetProvider.hpp — loads territories from a bundled/​on-disk JSON file.
//
// This is the application's "source of truth" provider. It ships with the real
// administrative scaffolding of Uzbekistan (regions and the capital) whose names
// and approximate centroids are public geographic facts. Quality indicators are
// intentionally left absent in the bundled data so nothing is fabricated; they
// become populated only when a real dataset (government/statistical feed) is
// supplied at the configured path.
//
// JSON schema (see resources/data/uzbekistan.json and docs/DATA_SCHEMA.md):
//   {
//     "source": "…",
//     "districts": [
//       {
//         "id": "…", "parentId": "…",
//         "name": {"ru": "…", "uz": "…", "en": "…"},
//         "level": "country|region|city|district|mahalla",
//         "center": {"lat": 0.0, "lng": 0.0},
//         "boundary": [[lat,lng], …],            // optional
//         "population": {"value": 0, "source": "…"},   // optional
//         "indicators": {                              // all optional
//           "safety": {"value": 0..100, "source": "…"}, …
//         },
//         "recommendations": ["…"]                     // optional
//       }
//     ]
//   }

#pragma once

#include <QString>

#include "geobiz/data/IDataProvider.hpp"

namespace geobiz::data {

class LocalDatasetProvider final : public IDataProvider {
public:
    /// `path` may be a Qt resource path (":/data/uzbekistan.json") or a real
    /// filesystem path. The filesystem is checked first so a deployment can ship
    /// updated open-data without rebuilding.
    explicit LocalDatasetProvider(QString path);

    [[nodiscard]] std::string name() const override;
    [[nodiscard]] DatasetResult load() override;

private:
    QString path_;
};

}  // namespace geobiz::data
