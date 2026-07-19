// SPDX-License-Identifier: MIT
//
// Metric.hpp — a single, optionally-present numeric indicator with provenance.
//
// The core project rule is "do not invent information". Every quantitative
// indicator therefore carries three things:
//   * whether a value is actually available (`has_value`),
//   * the value itself, normalised to a 0..100 scale, and
//   * the source it came from, so the UI can attribute the number honestly.
//
// A `Metric` with no value renders as "Недостаточно данных" everywhere.

#pragma once

#include <optional>
#include <string>
#include <utility>

namespace geobiz::core {

/// A normalised indicator in the closed range [0, 100] plus its data source.
///
/// This is a thin wrapper over std::optional<double> that additionally records
/// provenance and clamps values to the valid range. It is intentionally cheap
/// to copy so it can live directly inside the District aggregate.
class Metric {
public:
    Metric() = default;

    /// Constructs a populated metric. The score is clamped into [0, 100].
    Metric(double score, std::string source)
        : value_(clamp(score)), source_(std::move(source)) {}

    /// Named factory for an empty (missing) metric.
    [[nodiscard]] static Metric missing() { return Metric{}; }

    [[nodiscard]] bool hasValue() const noexcept { return value_.has_value(); }

    /// Returns the score, or the provided fallback when the metric is missing.
    [[nodiscard]] double valueOr(double fallback) const noexcept {
        return value_.value_or(fallback);
    }

    /// Precondition: hasValue() == true.
    [[nodiscard]] double value() const { return value_.value(); }

    [[nodiscard]] const std::string& source() const noexcept { return source_; }

private:
    static constexpr double clamp(double v) noexcept {
        if (v < 0.0) return 0.0;
        if (v > 100.0) return 100.0;
        return v;
    }

    std::optional<double> value_;
    std::string           source_;
};

}  // namespace geobiz::core
