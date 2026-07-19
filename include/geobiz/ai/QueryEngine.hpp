// SPDX-License-Identifier: MIT
//
// QueryEngine.hpp — the local, offline AI assistant.
//
// This is a fully on-device natural-language query engine (no network, no cloud
// model). It interprets Russian business questions such as:
//   • "Где лучше открыть кофейню?"
//   • "Где меньше конкуренция?"
//   • "Самые богатые районы Ташкента"
//   • "Лучшие районы для ресторана"
//   • "Самые безопасные районы"
// and answers them by ranking the districts already present in the repository.
//
// Design: a transparent intent classifier maps the query to (a) an optional
// geographic scope and (b) a ranking objective expressed over the district
// indicators. It NEVER invents an answer — if no district carries the indicator
// needed to answer, it says so ("Недостаточно данных").

#pragma once

#include <string>
#include <vector>

#include "geobiz/core/District.hpp"

namespace geobiz::ai {

/// One ranked answer row.
struct Suggestion {
    std::string districtId;
    std::string name;
    double      score = 0.0;      ///< The ranking score used (0..100).
    std::string rationaleRu;      ///< Why this district was suggested.
};

/// The engine's structured response to a query.
struct Answer {
    std::string             summaryRu;    ///< Natural-language headline answer.
    std::vector<Suggestion> suggestions;  ///< Ranked districts (may be empty).
    bool                    sufficientData = true;
};

/// The recognised objective behind a query.
enum class Intent {
    Unknown,
    OpenCoffeeShop,     ///< Где открыть кофейню.
    OpenRestaurant,     ///< Лучшие районы для ресторана.
    OpenBusinessGeneric,///< Общий бизнес / где открыть бизнес.
    LowCompetition,     ///< Где меньше конкуренция.
    Wealthiest,         ///< Самые богатые районы.
    Safest,             ///< Самые безопасные районы.
    BestEcology,        ///< Лучшая экология.
    BestInfrastructure, ///< Лучшая инфраструктура.
};

/// Parsed query: intent plus an optional geographic scope (e.g. "Ташкент").
/// The scope is captured in both Cyrillic (to match localized names) and Latin
/// (to match the transliterated ids used in the dataset), because a query token
/// like "Ташкента" must still match the id "uz.tashkent-city".
struct ParsedQuery {
    Intent      intent = Intent::Unknown;
    std::string scopeCyrillic;  ///< Lower-cased Cyrillic scope, empty ⇒ country.
    std::string scopeLatin;     ///< Latin/id form of the same scope, may be empty.

    [[nodiscard]] bool hasScope() const { return !scopeCyrillic.empty(); }
};

/// Stateless engine operating over a snapshot of districts.
class QueryEngine {
public:
    explicit QueryEngine(const std::vector<core::District>& districts)
        : districts_(districts) {}

    /// Interprets a raw Russian query into intent + scope.
    [[nodiscard]] static ParsedQuery parse(const std::string& rawQuery);

    /// Answers a raw query end to end.
    [[nodiscard]] Answer ask(const std::string& rawQuery, std::size_t topN = 5) const;

private:
    const std::vector<core::District>& districts_;
};

}  // namespace geobiz::ai
