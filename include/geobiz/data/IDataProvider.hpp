// SPDX-License-Identifier: MIT
//
// IDataProvider.hpp — abstraction over every source of territory data.
//
// The application deliberately depends only on this interface, never on a
// concrete source. That lets us plug in, side by side:
//   * a bundled local dataset (administrative scaffolding of Uzbekistan),
//   * Google Places / Geocoding enrichment,
//   * open government / statistical / ecological / geological feeds.
//
// A provider must NEVER fabricate indicators. When it has no data for a field it
// returns a `Metric::missing()` (or omits the district entirely).

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "geobiz/core/District.hpp"

namespace geobiz::data {

/// Result of a bulk load. `sourceName` identifies the provider for attribution
/// in the UI; `districts` may be empty (a perfectly valid "no data" outcome).
struct DatasetResult {
    std::string                        sourceName;
    std::vector<core::District>        districts;
    bool                               ok = false;
    std::string                        message;  ///< Human-readable status / error.
};

/// Interface implemented by every data source.
class IDataProvider {
public:
    virtual ~IDataProvider() = default;

    /// A short, stable identifier for the provider (used for attribution/logs).
    [[nodiscard]] virtual std::string name() const = 0;

    /// Loads all territories this provider knows about. Implementations should be
    /// safe to call off the GUI thread; they must not block indefinitely.
    [[nodiscard]] virtual DatasetResult load() = 0;

    /// Optionally enriches an already-loaded set of districts in place (e.g. an
    /// enrichment provider that adds Places-derived service metrics). The default
    /// implementation is a no-op so pure "source of truth" providers need not
    /// implement it.
    virtual void enrich(std::vector<core::District>& /*districts*/) {}
};

using DataProviderPtr = std::shared_ptr<IDataProvider>;

}  // namespace geobiz::data
