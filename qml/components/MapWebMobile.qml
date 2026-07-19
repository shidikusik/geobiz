// SPDX-License-Identifier: MIT
// MapWebMobile.qml — mobile map backend using Qt WebView (Android/iOS).
//
// QtWebView wraps the platform's native web view and therefore cannot use the
// QWebChannel transport that the desktop backend relies on. Instead:
//   * C++ → JS: we connect to the same App.mapBridge signals and push data with
//     runJavaScript(), reusing the identical MapBridge flow as desktop.
//   * JS → C++: map.js navigates to custom "geobiz://…" URLs which we intercept
//     in onLoadingChanged and forward to the native bridge.
import QtQuick
import QtWebView
import GeoBiz

Item {
    anchors.fill: parent

    WebView {
        id: web
        anchors.fill: parent
        url: App.mapPageUrl()

        // JS → C++ : intercept the custom-scheme "navigations" emitted by map.js.
        onLoadingChanged: function(request) {
            var u = request.url.toString()
            if (u.indexOf("geobiz://") !== 0)
                return

            if (u.indexOf("geobiz://ready") === 0) {
                // The page is ready — kick off the standard bridge flow which
                // emits configReady + renderDistricts, handled below.
                App.mapBridge.mapReady()
            } else if (u.indexOf("geobiz://district/") === 0) {
                var id = decodeURIComponent(u.substring("geobiz://district/".length))
                App.mapBridge.districtClicked(id)
            } else if (u.indexOf("geobiz://error/") === 0) {
                var msg = decodeURIComponent(u.substring("geobiz://error/".length))
                App.mapBridge.logError(msg)
            }
        }
    }

    // C++ → JS : mirror the MapBridge signals into injected JavaScript calls.
    Connections {
        target: App.mapBridge
        function onConfigReady(apiKey) {
            web.runJavaScript("window.geobizMobile.configReady(" +
                              JSON.stringify(apiKey) + ")")
        }
        function onRenderDistricts(geoJson) {
            web.runJavaScript("window.geobizMobile.renderDistricts(" +
                              JSON.stringify(geoJson) + ")")
        }
        function onFlyTo(lat, lng, zoom) {
            web.runJavaScript("window.geobizMobile.flyTo(" + lat + "," + lng +
                              "," + zoom + ")")
        }
        function onHighlightDistrict(id) {
            web.runJavaScript("window.geobizMobile.highlightDistrict(" +
                              JSON.stringify(id) + ")")
        }
    }
}
