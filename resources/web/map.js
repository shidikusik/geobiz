// SPDX-License-Identifier: MIT
//
// map.js — Google Maps ⇄ native bridge glue for GeoBiz Uzbekistan.
//
// Responsibilities:
//   * connect to the native `geobiz` bridge object over QWebChannel,
//   * dynamically load the official Google Maps JS API (Maps + Places +
//     Geocoding) with the key handed over by the native side,
//   * draw the districts (polygons or markers) with their zone colours,
//   * report clicks and errors back to native.
//
// The page keeps only presentation state; all data and classification come from
// the C++ side. No maps other than Google are used.

"use strict";

(function () {
  // Approximate geographic centre of Uzbekistan and a country-framing zoom.
  const UZ_CENTER = { lat: 41.377491, lng: 64.585262 };
  const UZ_ZOOM = 6;

  let bridge = null;       // native geobiz object
  let map = null;          // google.maps.Map
  let shapesById = {};      // id -> { overlay, base style }
  let highlightedId = null;
  let pendingPayload = null; // districts payload received before Maps was ready
  let mapsReady = false;

  function showOverlay(text) {
    const o = document.getElementById("overlay");
    document.getElementById("overlay-text").textContent = text;
    o.classList.add("show");
  }
  function hideOverlay() {
    document.getElementById("overlay").classList.remove("show");
  }

  // ---------------------------------------------------------------------------
  // Transport abstraction.
  //
  // On desktop (QtWebEngine) `bridge` is a live QWebChannel object with signals
  // and invokable methods. On mobile (QtWebView) there is no channel, so JS→C++
  // messages are sent by navigating to custom "geobiz://…" URLs which the native
  // side intercepts, and C++→JS calls arrive through window.geobizMobile.*.
  // ---------------------------------------------------------------------------
  let mobileMode = false;

  function emitReady() {
    if (bridge && bridge.mapReady) bridge.mapReady();
    else window.location.href = "geobiz://ready";
  }
  function emitClick(id) {
    if (bridge && bridge.districtClicked) bridge.districtClicked(id);
    else window.location.href = "geobiz://district/" + encodeURIComponent(id);
  }

  function reportError(message) {
    showOverlay(message);
    if (bridge && bridge.logError) bridge.logError(message);
    else if (mobileMode) window.location.href =
      "geobiz://error/" + encodeURIComponent(message);
  }

  // -- Google Maps bootstrap --------------------------------------------------

  // Loads the Maps JS API for the given key. Google invokes window.__geobizInit
  // once the core library is ready (via the `callback` parameter).
  function loadGoogleMaps(apiKey) {
    if (!apiKey) {
      reportError("Ключ Google Maps API не задан.");
      return;
    }
    if (document.getElementById("gmaps-sdk")) return; // already loading/loaded

    window.__geobizInit = initMap;
    const script = document.createElement("script");
    script.id = "gmaps-sdk";
    script.async = true;
    script.defer = true;
    script.src =
      "https://maps.googleapis.com/maps/api/js" +
      "?key=" + encodeURIComponent(apiKey) +
      "&libraries=places,geometry" +
      "&language=ru&region=UZ" +
      "&callback=__geobizInit";
    script.onerror = function () {
      reportError("Не удалось загрузить Google Maps. Проверьте ключ и сеть.");
    };
    document.head.appendChild(script);
  }

  function initMap() {
    try {
      map = new google.maps.Map(document.getElementById("map"), {
        center: UZ_CENTER,
        zoom: UZ_ZOOM,
        mapTypeControl: true,
        streetViewControl: false,
        fullscreenControl: false,
        clickableIcons: false,
        gestureHandling: "greedy",
        restriction: {
          // Keep the viewport within/around Uzbekistan.
          latLngBounds: { north: 46.0, south: 37.0, west: 55.5, east: 74.0 },
          strictBounds: false,
        },
      });

      setupSearchBox();

      mapsReady = true;
      hideOverlay();

      // Draw anything that arrived before Maps finished loading.
      if (pendingPayload) {
        renderDistricts(pendingPayload);
        pendingPayload = null;
      }
    } catch (e) {
      reportError("Ошибка инициализации карты: " + e);
    }
  }

  // Google Places Autocomplete wired to the native search box in the map.
  function setupSearchBox() {
    const input = document.getElementById("searchbox");
    map.controls[google.maps.ControlPosition.TOP_LEFT].push(input);

    const autocomplete = new google.maps.places.Autocomplete(input, {
      componentRestrictions: { country: "uz" },
      fields: ["geometry", "name"],
    });
    autocomplete.bindTo("bounds", map);

    autocomplete.addListener("place_changed", function () {
      const place = autocomplete.getPlace();
      if (!place.geometry || !place.geometry.location) return;
      if (place.geometry.viewport) {
        map.fitBounds(place.geometry.viewport);
      } else {
        map.setCenter(place.geometry.location);
        map.setZoom(14);
      }
    });
  }

  // -- District rendering -----------------------------------------------------

  function clearShapes() {
    Object.keys(shapesById).forEach(function (id) {
      const s = shapesById[id];
      if (s && s.overlay) s.overlay.setMap(null);
    });
    shapesById = {};
  }

  // Renders the districts payload (JSON string). Polygons when a boundary is
  // present, otherwise a coloured circle marker at the centroid. Districts with
  // hasData === false are drawn in a neutral "no data" style and never coloured.
  function renderDistricts(payloadJson) {
    if (!mapsReady) {
      pendingPayload = payloadJson; // defer until Maps is ready
      return;
    }
    let data;
    try {
      data = JSON.parse(payloadJson);
    } catch (e) {
      reportError("Некорректные данные территорий.");
      return;
    }

    clearShapes();
    const districts = (data && data.districts) || [];

    districts.forEach(function (d) {
      const hasData = !!d.hasData;
      const fill = hasData ? d.color : "#9e9e9e";
      const fillOpacity = hasData ? d.fillOpacity : 0.12;
      const strokeColor = hasData ? d.color : "#9e9e9e";

      let overlay;
      if (Array.isArray(d.boundary) && d.boundary.length >= 3) {
        overlay = new google.maps.Polygon({
          paths: d.boundary.map(function (p) {
            return { lat: p.lat, lng: p.lng };
          }),
          strokeColor: strokeColor,
          strokeOpacity: 0.9,
          strokeWeight: 1.5,
          fillColor: fill,
          fillOpacity: fillOpacity,
          map: map,
          // "No data" polygons use a dashed outline to read as tentative.
          zIndex: hasData ? 2 : 1,
        });
      } else {
        overlay = new google.maps.Circle({
          center: { lat: d.center.lat, lng: d.center.lng },
          radius: 6000,
          strokeColor: strokeColor,
          strokeOpacity: 0.9,
          strokeWeight: 1.5,
          fillColor: fill,
          fillOpacity: fillOpacity,
          map: map,
          zIndex: hasData ? 2 : 1,
        });
      }

      overlay.addListener("click", function () {
        emitClick(d.id);
      });

      shapesById[d.id] = {
        overlay: overlay,
        baseWeight: 1.5,
        baseFillOpacity: fillOpacity,
      };
    });
  }

  // Emphasises one district (called from native on selection).
  function highlightDistrict(id) {
    if (highlightedId && shapesById[highlightedId]) {
      const prev = shapesById[highlightedId];
      prev.overlay.setOptions({
        strokeWeight: prev.baseWeight,
        fillOpacity: prev.baseFillOpacity,
      });
    }
    highlightedId = id;
    const cur = shapesById[id];
    if (cur) {
      cur.overlay.setOptions({
        strokeWeight: 4,
        fillOpacity: Math.min(0.85, cur.baseFillOpacity + 0.25),
      });
    }
  }

  function flyTo(lat, lng, zoom) {
    if (!map) return;
    map.panTo({ lat: lat, lng: lng });
    if (zoom) map.setZoom(zoom);
  }

  // -- Native bridge wiring ---------------------------------------------------

  function connectBridge() {
    showOverlay("Загрузка карты…");

    if (typeof qt !== "undefined" && qt.webChannelTransport &&
        typeof QWebChannel !== "undefined") {
      // Desktop: full QWebChannel transport.
      new QWebChannel(qt.webChannelTransport, function (channel) {
        bridge = channel.objects.geobiz;
        bridge.configReady.connect(loadGoogleMaps);
        bridge.renderDistricts.connect(renderDistricts);
        bridge.flyTo.connect(flyTo);
        bridge.highlightDistrict.connect(highlightDistrict);
        emitReady();
      });
      return;
    }

    // Mobile (QtWebView): expose the injection API the native side calls, then
    // announce readiness via the custom-scheme navigation.
    mobileMode = true;
    window.geobizMobile = {
      configReady: loadGoogleMaps,
      renderDistricts: renderDistricts,
      flyTo: flyTo,
      highlightDistrict: highlightDistrict,
    };
    emitReady();
  }

  window.addEventListener("load", connectBridge);
})();
