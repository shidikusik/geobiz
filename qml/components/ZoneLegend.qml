// SPDX-License-Identifier: MIT
// ZoneLegend.qml — a compact, collapsible legend explaining the colour zones.
// Fed by App.zoneLegend (built from the C++ zone table).
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import GeoBiz

Pane {
    id: root
    property var model: []
    property bool expanded: true

    padding: Theme.spacingS
    Material.background: Theme.surface2
    Material.elevation: 6
    width: 260

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spacingXs

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("Цветовые зоны")
                color: Theme.onSurface
                font.pixelSize: 14
                font.bold: true
                Layout.fillWidth: true
            }
            ToolButton {
                text: root.expanded ? "▾" : "▸"
                implicitWidth: 28; implicitHeight: 28
                onClicked: root.expanded = !root.expanded
            }
        }

        Repeater {
            model: root.expanded ? root.model : []
            delegate: RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spacingS
                Rectangle {
                    width: 14; height: 14; radius: 3
                    color: modelData.color
                    Layout.alignment: Qt.AlignTop
                    Layout.topMargin: 2
                }
                Label {
                    Layout.fillWidth: true
                    text: modelData.name + (modelData.showsWarning ? " ⚠" : "")
                    color: Theme.onSurfaceVariant
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
