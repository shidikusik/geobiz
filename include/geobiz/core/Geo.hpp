// SPDX-License-Identifier: MIT
//
// Geo.hpp — lightweight geographic primitives shared across the core layer.
//
// These types intentionally avoid any Qt or GIS-library dependency so that the
// pure domain logic (classification, the AI query engine, data parsing) can be
// unit-tested and reused on any platform, including headless CI.

#pragma once

#include <cmath>
#include <numbers>
#include <utility>
#include <vector>

namespace geobiz::core {

/// A WGS-84 geographic coordinate (degrees).
struct LatLng {
    double lat = 0.0;
    double lng = 0.0;

    [[nodiscard]] constexpr bool isValid() const noexcept {
        return lat >= -90.0 && lat <= 90.0 && lng >= -180.0 && lng <= 180.0;
    }
};

/// An axis-aligned geographic bounding box.
struct BBox {
    LatLng southWest{90.0, 180.0};
    LatLng northEast{-90.0, -180.0};

    void extend(const LatLng& p) noexcept {
        southWest.lat = std::min(southWest.lat, p.lat);
        southWest.lng = std::min(southWest.lng, p.lng);
        northEast.lat = std::max(northEast.lat, p.lat);
        northEast.lng = std::max(northEast.lng, p.lng);
    }

    [[nodiscard]] LatLng center() const noexcept {
        return {(southWest.lat + northEast.lat) * 0.5,
                (southWest.lng + northEast.lng) * 0.5};
    }

    [[nodiscard]] bool isValid() const noexcept {
        return southWest.lat <= northEast.lat && southWest.lng <= northEast.lng;
    }
};

/// A polygon boundary: an ordered ring of coordinates. May be empty when only a
/// centroid is known for the territory (in which case the map draws a marker).
using Polygon = std::vector<LatLng>;

/// Computes the bounding box of a polygon ring.
[[nodiscard]] inline BBox boundsOf(const Polygon& ring) noexcept {
    BBox box;
    for (const auto& p : ring) {
        box.extend(p);
    }
    return box;
}

/// Great-circle distance in kilometres (Haversine). Used by the AI query engine
/// for "near me" style ranking when a reference point is available.
[[nodiscard]] inline double distanceKm(const LatLng& a, const LatLng& b) noexcept {
    constexpr double kEarthRadiusKm = 6371.0088;
    // Use std::numbers::pi rather than the non-standard M_PI, which MSVC does
    // not define (and which strict-ISO GCC/Clang also hide) without extra macros.
    constexpr double kDegToRad = std::numbers::pi / 180.0;
    const double dLat = (b.lat - a.lat) * kDegToRad;
    const double dLng = (b.lng - a.lng) * kDegToRad;
    const double lat1 = a.lat * kDegToRad;
    const double lat2 = b.lat * kDegToRad;
    const double h = std::sin(dLat / 2) * std::sin(dLat / 2) +
                     std::cos(lat1) * std::cos(lat2) *
                         std::sin(dLng / 2) * std::sin(dLng / 2);
    return 2.0 * kEarthRadiusKm * std::asin(std::min(1.0, std::sqrt(h)));
}

}  // namespace geobiz::core
