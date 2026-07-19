// SPDX-License-Identifier: MIT
//
// Zone.hpp — classification of a territory into one of the GeoBiz colour zones.
//
// Each administrative territory (region, city, district or mahalla) can be
// painted on the map with exactly one *primary* colour zone. The zone is a
// deliberately small, closed vocabulary so that the meaning of every colour is
// unambiguous both for the UI legend and for the local AI assistant.
//
// IMPORTANT (project policy): a zone must never be *invented*. A territory is
// only assigned a colour when the underlying data layer provides enough signal
// to justify it (see Classifier.hpp). When the data is insufficient the zone is
// `Unknown` and the territory is intentionally left uncoloured.

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace geobiz::core {

/// The closed set of colour zones understood by the whole application.
///
/// The numeric values are stable and are used as an on-the-wire contract with
/// the JavaScript map layer (see resources/web/map.js). Do not renumber.
enum class ZoneType : std::uint8_t {
    Unknown = 0,  ///< Not enough data — territory stays uncoloured ("Недостаточно данных").
    Green   = 1,  ///< Wealthy, developed infrastructure, safe, good ecology.
    Purple  = 2,  ///< Business-class: modern housing, cafes, restaurants, high service.
    Blue    = 3,  ///< Budget housing, low property prices, limited infrastructure.
    Gray    = 4,  ///< High crime / unfavourable social environment (shows a warning).
    Yellow  = 5,  ///< Health hazard: power lines, landfills, air pollution, noise.
    Orange  = 6,  ///< Construction risk: unstable ground, subsidence, difficult geology.
    Red     = 7,  ///< Life-threatening: technogenic risk, hazardous industry, emergencies.
};

/// Static, human-readable metadata describing a single zone.
struct ZoneInfo {
    ZoneType         type;
    std::string_view id;        ///< Stable machine identifier, e.g. "green".
    std::string_view nameRu;    ///< Display name (Russian) — primary UI language.
    std::string_view cssColor;  ///< Fill colour handed to Google Maps polygons (#RRGGBB).
    double           fillOpacity;  ///< Suggested polygon fill opacity [0..1].
    bool             showsWarning;  ///< Whether opening the district shows a safety warning.
    std::string_view descriptionRu;  ///< Short description used in the legend / tooltips.
};

namespace detail {

// The single source of truth for every zone's presentation. Kept as a constexpr
// table so it can be consumed from both C++ and (via serialisation) the QML/JS
// layers without duplicating the mapping.
inline constexpr std::array<ZoneInfo, 8> kZoneTable{{
    {ZoneType::Unknown, "unknown", "Недостаточно данных", "#9e9e9e", 0.0, false,
     "Недостаточно данных для классификации территории."},
    {ZoneType::Green, "green", "Зелёная зона", "#2e7d32", 0.45, false,
     "Богатые районы: развитая инфраструктура, высокий уровень жизни, "
     "безопасность, хорошая экология, современные ЖК и парки."},
    {ZoneType::Purple, "purple", "Фиолетовая зона", "#6a1b9a", 0.45, false,
     "Районы бизнес-класса: современные дома, кафе, рестораны, высокий "
     "уровень сервиса и развитая коммерция."},
    {ZoneType::Blue, "blue", "Синяя зона", "#1565c0", 0.40, false,
     "Бюджетное жильё: низкая стоимость недвижимости, ограниченная "
     "инфраструктура, недорогие магазины, сервис ниже среднего."},
    {ZoneType::Gray, "gray", "Серая зона", "#455a64", 0.55, true,
     "Высокий уровень преступности и неблагоприятная социальная обстановка. "
     "Низкий уровень безопасности."},
    {ZoneType::Yellow, "yellow", "Жёлтая зона", "#f9a825", 0.50, true,
     "Опасность для здоровья: рядом ЛЭП, свалки, загрязнение воздуха, "
     "промышленные предприятия, высокий уровень шума."},
    {ZoneType::Orange, "orange", "Оранжевая зона", "#ef6c00", 0.50, true,
     "Строительные проблемы: неустойчивый грунт, риск просадки, сложные "
     "инженерно-геологические условия, вероятность повреждения зданий."},
    {ZoneType::Red, "red", "Красная зона", "#c62828", 0.55, true,
     "Опасные для жизни территории: зоны техногенного риска, опасные "
     "производственные объекты, высокий риск чрезвычайных ситуаций."},
}};

}  // namespace detail

/// Returns the static metadata for a zone. Always valid — `Unknown` for any
/// out-of-range value so callers never have to guard against a null result.
[[nodiscard]] constexpr const ZoneInfo& zoneInfo(ZoneType type) noexcept {
    const auto index = static_cast<std::size_t>(type);
    if (index < detail::kZoneTable.size()) {
        return detail::kZoneTable[index];
    }
    return detail::kZoneTable[0];  // Unknown
}

/// The full zone table, useful for building the UI legend.
[[nodiscard]] constexpr const std::array<ZoneInfo, 8>& allZones() noexcept {
    return detail::kZoneTable;
}

/// Parses a stable zone id ("green", "red", ...) back into an enum value.
/// Returns `ZoneType::Unknown` for anything unrecognised.
[[nodiscard]] constexpr ZoneType zoneFromId(std::string_view id) noexcept {
    for (const auto& info : detail::kZoneTable) {
        if (info.id == id) {
            return info.type;
        }
    }
    return ZoneType::Unknown;
}

}  // namespace geobiz::core
