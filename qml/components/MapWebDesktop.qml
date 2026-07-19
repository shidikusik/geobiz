// SPDX-License-Identifier: MIT
// MapWebDesktop.qml — desktop map backend using Qt WebEngine + QWebChannel.
//
// The real Google Maps JS API runs inside qrc:/web/map.html. A QWebChannel
// publishes the native `App.mapBridge` object to the page as "geobiz"; the page
// uses the standard qwebchannel.js transport (qt.webChannelTransport).
import QtQuick
import QtWebEngine
import QtWebChannel
import GeoBiz

Item {
    anchors.fill: parent

    WebChannel {
        id: channel
    }

    WebEngineView {
        id: web
        anchors.fill: parent
        url: "qrc:/web/map.html"
        webChannel: channel
        backgroundColor: Theme.surface

        settings.javascriptEnabled: true
        settings.localContentCanAccessRemoteUrls: true
        settings.localContentCanAccessFileUrls: true

        onLoadingChanged: function(info) {
            if (info.status === WebEngineView.LoadFailedStatus && App.mapBridge) {
                App.mapBridge.logError(qsTr("Не удалось загрузить страницу карты."))
            }
        }

        Component.onCompleted: {
            // Publish the native bridge object to the page's JS as `geobiz`.
            channel.registerObject("geobiz", App.mapBridge)
        }
    }
}
