// SPDX-License-Identifier: MIT
// IndicatorRow.qml — one labelled 0..100 indicator with a progress bar, or the
// honest "Недостаточно данных" text when the value is not numeric.
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import GeoBiz

ColumnLayout {
    id: root
    property string label: ""
    property var value: ""   // numeric string, or "Недостаточно данных"

    // A value is numeric when it parses to a finite number.
    readonly property bool numeric: value !== "" && !isNaN(Number(value))
    readonly property real numValue: numeric ? Number(value) : 0

    Layout.fillWidth: true
    spacing: 2

    RowLayout {
        Layout.fillWidth: true
        Label {
            text: root.label
            color: Theme.onSurface
            font.pixelSize: 13
            Layout.fillWidth: true
        }
        Label {
            text: root.numeric ? root.numValue.toFixed(0) : qsTr("Недостаточно данных")
            color: root.numeric ? Theme.primary : Theme.onSurfaceVariant
            font.pixelSize: 13
            font.bold: root.numeric
        }
    }

    ProgressBar {
        Layout.fillWidth: true
        visible: root.numeric
        from: 0; to: 100
        value: root.numValue
        Material.accent: {
            if (root.numValue >= 70) return "#2e7d32"
            if (root.numValue >= 45) return "#f9a825"
            return "#c62828"
        }
    }
}
