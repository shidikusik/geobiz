// SPDX-License-Identifier: MIT
//
// Classifier.hpp — turns a District's indicators into a colour zone + rating.
//
// The classifier is the one place in the codebase that decides which colour a
// territory receives. Its rules are transparent and deterministic so the choice
// can always be explained to the user. Crucially, it refuses to classify (leaves
// `ZoneType::Unknown`) when the data is insufficient — see `classify()`.

#pragma once

#include "geobiz/core/District.hpp"

namespace geobiz::core {

/// Tunable thresholds for the classification rules. Exposed as a struct so they
/// can be overridden from configuration/tests instead of being magic numbers.
struct ClassifierConfig {
    // A district needs at least this many of the positive indicators present to
    // be eligible for a (non-hazard) colour at all.
    int minPositiveIndicators = 3;

    // Risk thresholds (0..100). At or above these a hazard zone is forced, in
    // priority order red > orange > yellow > gray.
    double technogenicRiskThreshold = 60.0;  // → Red
    double constructionRiskThreshold = 60.0;  // → Orange
    double healthHazardThreshold = 60.0;      // → Yellow
    double crimeThreshold = 60.0;             // → Gray

    // Positive-zone cut-offs on the composite score (0..100).
    double greenScoreThreshold = 72.0;   // wealthy + safe + green
    double purpleScoreThreshold = 58.0;  // business class
    // below purple ⇒ Blue (budget), provided data is sufficient.
};

/// The outcome of classifying a district.
struct Classification {
    ZoneType zone = ZoneType::Unknown;
    double   rating = 0.0;   ///< Overall 0..100 rating (0 when unknown).
    bool     sufficientData = false;
    std::string reasonRu;    ///< Short explanation of why this zone was chosen.
};

/// Computes the overall 0..100 rating from the positive indicators that are
/// present. Returns std::nullopt when too few indicators are available.
[[nodiscard]] std::optional<double> compositeRating(
    const Indicators& indicators, const ClassifierConfig& cfg = {});

/// Classifies a district. Hazard/risk zones take priority over quality zones,
/// matching the product spec (a life-threatening area is red even if it is also
/// wealthy). Leaves the zone `Unknown` when data is insufficient.
[[nodiscard]] Classification classify(const District& district,
                                      const ClassifierConfig& cfg = {});

/// Applies `classify()` to a district in place (sets zone, hasSufficientData and
/// appends any newly derived recommendations without duplicating existing ones).
void applyClassification(District& district, const ClassifierConfig& cfg = {});

/// Generates data-derived recommendations for a district. Only produces advice
/// backed by indicators that actually have values — never invented.
[[nodiscard]] std::vector<std::string> deriveRecommendations(
    const District& district);

}  // namespace geobiz::core
