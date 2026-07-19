// SPDX-License-Identifier: MIT
//
// MapBridge.hpp — the C++⇄JavaScript bridge for the Google Maps layer.
//
// A single instance of this QObject is published on a QWebChannel and shared
// with the page hosted in QtWebEngine (resources/web/map.html + map.js). The
// real Google Maps JavaScript API runs inside that page; this class is the only
// contract between it and the native application:
//
//   JS → C++ (Q_INVOKABLE):  mapReady(), districtClicked(id), logError(msg)
//   C++ → JS (signals):      configReady(key,...), renderDistricts(json),
//                            flyTo(lat,lng,zoom), highlightDistrict(id)
//
// Keeping the surface this small makes the web layer trivially replaceable and
// keeps all business logic on the native side.

#pragma once

#include <QObject>
#include <QString>

namespace geobiz::map {

class MapBridge : public QObject {
    Q_OBJECT
public:
    explicit MapBridge(QObject* parent = nullptr);

    /// Sets the Google Maps API key handed to the page on initialisation.
    void setApiKey(QString key);

    /// Provides the serialised district payload (GeoJSON-like, see map.js) that
    /// the page should render. Cached so a page reload can re-request it.
    void setDistrictPayload(QString json);

    // ---- Called from JavaScript --------------------------------------------

    /// The page finished loading the Google Maps script and is ready to receive
    /// data. Triggers configReady() + renderDistricts() with the cached payload.
    Q_INVOKABLE void mapReady();

    /// A territory polygon/marker was clicked in the map.
    Q_INVOKABLE void districtClicked(const QString& id);

    /// The map viewport changed (used for lazy/priority loading heuristics).
    Q_INVOKABLE void viewportChanged(double south, double west, double north,
                                     double east, int zoom);

    /// The JS layer reports an error (e.g. Maps failed to load / bad key).
    Q_INVOKABLE void logError(const QString& message);

signals:
    // ---- Emitted to JavaScript ---------------------------------------------
    void configReady(const QString& apiKey);
    void renderDistricts(const QString& geoJson);
    void flyTo(double lat, double lng, int zoom);
    void highlightDistrict(const QString& id);

    // ---- Emitted to the native UI ------------------------------------------
    void districtSelected(const QString& id);
    void mapErrorOccurred(const QString& message);
    void ready();

public slots:
    /// Native-side request to centre the map on a coordinate.
    void focusOn(double lat, double lng, int zoom);
    /// Native-side request to highlight a specific district.
    void highlight(const QString& id);

private:
    QString apiKey_;
    QString payload_;
    bool    pageReady_ = false;
};

}  // namespace geobiz::map
