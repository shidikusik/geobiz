// SPDX-License-Identifier: MIT
// Main.qml — application shell: adaptive layout combining the map, the browse /
// search side panel, the district info panel and the AI assistant.
//
// Layout adapts at Theme.compactWidth: wide screens show a persistent side panel
// next to the map; narrow screens (phones) show the map full-bleed with the
// panels presented as overlay drawers.
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import GeoBiz

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 820
    minimumWidth: 360
    minimumHeight: 560
    title: qsTr("GeoBiz Uzbekistan")

    Material.theme: Theme.dark ? Material.Dark : Material.Light
    Material.primary: Theme.primary
    Material.accent: Theme.primary
    Material.background: Theme.surface

    readonly property bool compact: width < Theme.compactWidth

    // ---- Top app bar --------------------------------------------------------
    header: ToolBar {
        Material.background: Theme.surface2
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.spacingM
            anchors.rightMargin: Theme.spacingS
            spacing: Theme.spacingS

            ToolButton {
                visible: window.compact
                text: "☰" // hamburger
                onClicked: sideDrawer.open()
            }
            Label {
                text: qsTr("GeoBiz Uzbekistan")
                color: Theme.onSurface
                font.pixelSize: 20
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            Label {
                text: App.statusMessage
                color: Theme.onSurfaceVariant
                font.pixelSize: 12
                visible: !window.compact
                elide: Text.ElideRight
                Layout.maximumWidth: 320
            }
            ToolButton {
                text: qsTr("ИИ")
                onClicked: aiDrawer.open()
            }
            ToolButton {
                text: Theme.dark ? "☀" : "☽" // sun / moon
                onClicked: Theme.dark = !Theme.dark
            }
        }
    }

    // ---- Central content ----------------------------------------------------
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Persistent side panel on wide screens.
        Pane {
            visible: !window.compact
            Layout.preferredWidth: 340
            Layout.fillHeight: true
            padding: 0
            Material.background: Theme.surface1
            BrowsePanel {
                anchors.fill: parent
                onDistrictActivated: function(id) { App.selectDistrict(id) }
            }
        }

        // Map + floating overlays.
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            MapView {
                id: mapView
                anchors.fill: parent
                onRequestApiKey: apiKeyDialog.open()
            }

            // Zone legend, bottom-left, floating over the map.
            ZoneLegend {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: Theme.spacingM
                model: App.zoneLegend
            }

            // District info card, slides in from the right when a district is
            // selected. On compact screens it becomes a bottom sheet.
            DistrictInfoPanel {
                id: infoPanel
                width: window.compact ? parent.width
                                      : Math.min(400, parent.width - Theme.spacingL)
                anchors.right: window.compact ? undefined : parent.right
                anchors.left: window.compact ? parent.left : undefined
                anchors.top: window.compact ? undefined : parent.top
                anchors.bottom: parent.bottom
                anchors.margins: window.compact ? 0 : Theme.spacingM
                visible: App.hasSelection
                onClosed: App.clearSelection()
            }
        }
    }

    // ---- Overlay drawers (compact) -----------------------------------------
    Drawer {
        id: sideDrawer
        width: Math.min(360, window.width * 0.9)
        height: window.height
        edge: Qt.LeftEdge
        Material.background: Theme.surface1
        BrowsePanel {
            anchors.fill: parent
            onDistrictActivated: function(id) {
                App.selectDistrict(id); sideDrawer.close()
            }
        }
    }

    Drawer {
        id: aiDrawer
        width: Math.min(420, window.width * 0.95)
        height: window.height
        edge: Qt.RightEdge
        Material.background: Theme.surface1
        AiAssistantPanel {
            anchors.fill: parent
            onSuggestionActivated: function(id) {
                App.selectDistrict(id); if (window.compact) aiDrawer.close()
            }
        }
    }

    // ---- Dialogs ------------------------------------------------------------
    ApiKeyDialog {
        id: apiKeyDialog
        anchors.centerIn: parent
        onAccepted: function(key) { App.setApiKey(key) }
    }

    // Surface backend errors as a transient snackbar-like popup.
    Connections {
        target: App
        function onErrorOccurred(message) {
            errorPopup.text = message
            errorPopup.open()
        }
    }
    Popup {
        id: errorPopup
        property alias text: errorLabel.text
        anchors.centerIn: Overlay.overlay
        modal: false
        Material.background: Theme.error
        Label {
            id: errorLabel
            color: "#ffffff"
            wrapMode: Text.WordWrap
        }
        Timer {
            running: errorPopup.visible
            interval: 4000
            onTriggered: errorPopup.close()
        }
    }
}
