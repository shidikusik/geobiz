// SPDX-License-Identifier: MIT
//
// District.hpp — the central domain aggregate.
//
// A "District" here is a generic administrative territory at any level of the
// Uzbekistan hierarchy: region (viloyat), city, district (tuman) or mahalla.
// The level is captured by `AdminLevel`. Every quantitative field is a `Metric`
// so that missing data is represented explicitly rather than as a fake zero.

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "geobiz/core/Geo.hpp"
#include "geobiz/core/Metric.hpp"
#include "geobiz/core/Zone.hpp"

namespace geobiz::core {

/// Administrative level within the Uzbekistan territorial hierarchy.
enum class AdminLevel {
    Country,   ///< O'zbekiston (root).
    Region,    ///< Viloyat / Republic of Karakalpakstan / Tashkent city.
    City,      ///< Shahar.
    District,  ///< Tuman.
    Mahalla,   ///< Neighbourhood community.
};

[[nodiscard]] std::string_view adminLevelId(AdminLevel level) noexcept;
[[nodiscard]] std::string_view adminLevelNameRu(AdminLevel level) noexcept;
[[nodiscard]] std::optional<AdminLevel> adminLevelFromId(std::string_view id) noexcept;

/// A bundle of the seven indicators that drive both the rating and the colour
/// classification. Each may be independently missing.
struct Indicators {
    Metric safety;          ///< Безопасность (higher = safer).
    Metric ecology;         ///< Экология (higher = cleaner).
    Metric infrastructure;  ///< Инфраструктура.
    Metric service;         ///< Уровень сервиса.
    Metric businessPotential;  ///< Бизнес-потенциал.
    Metric wealth;          ///< Уровень жизни / достаток.

    // Risk indicators — higher means *more* risk. Presence of a high value can
    // force a hazard zone (yellow/orange/red) regardless of the positive ones.
    Metric healthHazard;      ///< Опасность для здоровья (ЛЭП, свалки, воздух...).
    Metric constructionRisk;  ///< Строительные / геологические риски.
    Metric technogenicRisk;   ///< Техногенный риск / ЧС.
    Metric crime;             ///< Уровень преступности.
};

/// The full record for one territory.
struct District {
    std::string id;         ///< Stable unique identifier.
    std::string parentId;   ///< Parent territory id (empty for the country root).
    std::string nameRu;     ///< Primary display name.
    std::string nameUz;     ///< Uzbek name (optional).
    std::string nameEn;     ///< English name (optional).
    AdminLevel  level = AdminLevel::District;

    LatLng   center{};       ///< Representative point (always present).
    Polygon  boundary;       ///< Optional polygon ring; empty ⇒ marker only.

    // Population is a raw people-count (not a 0..100 metric), hence its own
    // optional field. Absent ⇒ "Недостаточно данных".
    std::optional<std::int64_t> populationCount;
    std::string                 populationSource;

    Indicators  indicators;   ///< The seven driving indicators.

    /// The classified colour zone. `Unknown` until a Classifier assigns it, and
    /// deliberately left `Unknown` when data is insufficient.
    ZoneType zone = ZoneType::Unknown;

    /// Overall 0..100 rating produced by the classifier (0 when insufficient).
    double rating = 0.0;

    /// Human-readable, data-derived recommendations (never fabricated — produced
    /// only from indicators that actually have values).
    std::vector<std::string> recommendations;

    /// True when at least the minimum indicator set required for classification
    /// is present. Mirrors Classifier logic and is cached for quick UI checks.
    bool hasSufficientData = false;

    [[nodiscard]] const std::string& displayName() const noexcept { return nameRu; }

    [[nodiscard]] BBox bounds() const noexcept {
        if (boundary.empty()) {
            BBox b;
            b.extend(center);
            return b;
        }
        return boundsOf(boundary);
    }
};

}  // namespace geobiz::core
