// SPDX-License-Identifier: MIT
#include "geobiz/map/MapBridge.hpp"

#include <utility>

namespace geobiz::map {

MapBridge::MapBridge(QObject* parent) : QObject(parent) {}

void MapBridge::setApiKey(QString key) { apiKey_ = std::move(key); }

void MapBridge::setDistrictPayload(QString json) {
    payload_ = std::move(json);
    // If the page is already up, push the fresh payload immediately.
    if (pageReady_) {
        emit renderDistricts(payload_);
    }
}

void MapBridge::mapReady() {
    pageReady_ = true;
    emit configReady(apiKey_);
    if (!payload_.isEmpty()) {
        emit renderDistricts(payload_);
    }
    emit ready();
}

void MapBridge::districtClicked(const QString& id) { emit districtSelected(id); }

void MapBridge::viewportChanged(double south, double west, double north,
                                double east, int zoom) {
    // Reserved hook for viewport-driven prioritised loading. Intentionally light
    // for now — the native side keeps the full set in memory and lets the JS
    // layer cull by bounds. Signature is stable so JS need not change later.
    Q_UNUSED(south)
    Q_UNUSED(west)
    Q_UNUSED(north)
    Q_UNUSED(east)
    Q_UNUSED(zoom)
}

void MapBridge::logError(const QString& message) {
    emit mapErrorOccurred(message);
}

void MapBridge::focusOn(double lat, double lng, int zoom) {
    emit flyTo(lat, lng, zoom);
}

void MapBridge::highlight(const QString& id) { emit highlightDistrict(id); }

}  // namespace geobiz::map
