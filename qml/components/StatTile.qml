// SPDX-License-Identifier: MIT
// StatTile.qml — a small labelled value tile used in the district info panel.
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import GeoBiz

Pane {
    id: root
    property string title: ""
    property string value: ""

    Layout.fillWidth: true
    padding: Theme.spacingS
    Material.background: Theme.surface2
    Material.elevation: 0

    ColumnLayout {
        anchors.fill: parent
        spacing: 2
        Label {
            text: root.title
            color: Theme.onSurfaceVariant
            font.pixelSize: 11
        }
        Label {
            text: root.value
            color: Theme.onSurface
            font.pixelSize: 18
            font.bold: true
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
    }
}
