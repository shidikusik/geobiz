// SPDX-License-Identifier: MIT
//
// DistrictSerializer.hpp — turns districts into the compact JSON the map draws.
//
// The JavaScript layer only needs geometry + presentation, not the full domain
// model, so we emit a lean payload: id, name, level, centre, optional boundary
// ring, the resolved zone id, its fill colour/opacity, and whether the territory
// has enough data to be coloured at all (uncoloured otherwise, per policy).

#pragma once

#include <vector>

#include <QString>

#include "geobiz/core/District.hpp"

namespace geobiz::map {

/// Serialises districts to a JSON string: {"districts":[ … ]}.
[[nodiscard]] QString buildMapPayload(const std::vector<core::District>& districts);

}  // namespace geobiz::map
