// SPDX-License-Identifier: MIT
#include "geobiz/map/DistrictSerializer.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include "geobiz/core/Zone.hpp"

namespace geobiz::map {

QString buildMapPayload(const std::vector<core::District>& districts) {
    QJsonArray items;

    for (const core::District& d : districts) {
        const core::ZoneInfo& zi = core::zoneInfo(d.zone);

        QJsonObject obj;
        obj["id"] = QString::fromStdString(d.id);
        obj["name"] = QString::fromStdString(d.nameRu);
        obj["level"] = QString::fromStdString(std::string(core::adminLevelId(d.level)));

        QJsonObject center;
        center["lat"] = d.center.lat;
        center["lng"] = d.center.lng;
        obj["center"] = center;

        // Only emit a boundary when we actually have one; the JS layer draws a
        // marker instead of a polygon when the ring is absent.
        if (!d.boundary.empty()) {
            QJsonArray ring;
            for (const core::LatLng& p : d.boundary) {
                QJsonObject pt;
                pt["lat"] = p.lat;
                pt["lng"] = p.lng;
                ring.append(pt);
            }
            obj["boundary"] = ring;
        }

        obj["zone"] = QString::fromStdString(std::string(zi.id));
        obj["color"] = QString::fromStdString(std::string(zi.cssColor));
        obj["fillOpacity"] = zi.fillOpacity;
        obj["showsWarning"] = zi.showsWarning;

        // Districts without sufficient data must not be coloured — the JS layer
        // uses this flag to render a neutral, hatched "no data" style.
        obj["hasData"] = d.hasSufficientData && d.zone != core::ZoneType::Unknown;

        items.append(obj);
    }

    QJsonObject root;
    root["districts"] = items;
    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

}  // namespace geobiz::map
