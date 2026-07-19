// SPDX-License-Identifier: MIT
// MapView.qml — platform-adaptive host for the Google Maps layer.
//
// Qt WebEngine (Chromium) exists only on desktop platforms; Android/iOS provide
// the native system web view through the Qt WebView module. This wrapper loads
// the correct backend at runtime:
//   * desktop  → MapWebDesktop.qml  (WebEngine + QWebChannel bridge)
//   * mobile   → MapWebMobile.qml   (WebView + injected-JS bridge)
// Both talk to the same C++ `App.mapBridge` and render the same map.html page,
// so all business logic stays shared.
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import GeoBiz

Item {
    id: root
    signal requestApiKey()

    // Shown when there is no API key configured yet.
    readonly property bool apiKeyMissing: !App.hasApiKey
    readonly property bool mobile: Qt.platform.os === "android" ||
                                   Qt.platform.os === "ios"

    Loader {
        anchors.fill: parent
        active: !root.apiKeyMissing
        source: root.mobile ? "MapWebMobile.qml" : "MapWebDesktop.qml"
    }

    // Loading indicator while the native data set streams in.
    BusyIndicator {
        anchors.centerIn: parent
        running: App.loading && !root.apiKeyMissing
        Material.accent: Theme.primary
    }

    // ---- API-key setup prompt ----------------------------------------------
    // Instead of silently loading a broken map we guide the user to configure a
    // Google Maps API key.
    Rectangle {
        anchors.fill: parent
        visible: root.apiKeyMissing
        color: Theme.surface

        Column {
            anchors.centerIn: parent
            width: Math.min(parent.width - Theme.spacingXl * 2, 480)
            spacing: Theme.spacingM

            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                text: "🗺️"
                font.pixelSize: 48
            }
            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("Требуется ключ Google Maps API")
                color: Theme.onSurface
                font.pixelSize: 22
                font.bold: true
            }
            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("Для отображения настоящей карты Google введите ключ " +
                           "API с включёнными Maps JavaScript API, Places API и " +
                           "Geocoding API.")
                color: Theme.onSurfaceVariant
                font.pixelSize: 14
            }
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Ввести ключ API")
                Material.background: Theme.primary
                Material.foreground: Theme.onPrimary
                onClicked: root.requestApiKey()
            }
        }
    }
}
