// SPDX-License-Identifier: MIT
// DistrictInfoPanel.qml — detailed card for the selected district.
// Reads App.selectedDistrict (a QVariantMap). Shows a safety warning banner for
// zones flagged with showsWarning (gray/yellow/orange/red).
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import GeoBiz

Pane {
    id: root
    signal closed()

    readonly property var d: App.selectedDistrict

    padding: 0
    Material.background: Theme.surface1
    Material.elevation: 8

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ---- Header with zone colour accent --------------------------------
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: headerCol.implicitHeight + Theme.spacingL
            color: root.d.hasData ? root.d.zoneColor : Theme.surface3

            ColumnLayout {
                id: headerCol
                anchors.fill: parent
                anchors.margins: Theme.spacingM
                spacing: Theme.spacingXs

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: root.d.name || ""
                        color: root.d.hasData ? "#ffffff" : Theme.onSurface
                        font.pixelSize: 22
                        font.bold: true
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }
                    ToolButton {
                        text: "✕"
                        onClicked: root.closed()
                        Material.foreground: root.d.hasData ? "#ffffff" : Theme.onSurface
                    }
                }
                Label {
                    text: (root.d.level || "") +
                          (root.d.hasData ? " · " + root.d.zoneName : "")
                    color: root.d.hasData ? "#f0f0f0" : Theme.onSurfaceVariant
                    font.pixelSize: 13
                }
            }
        }

        // ---- Scrollable body -----------------------------------------------
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: root.width
                spacing: Theme.spacingM

                // Safety / hazard warning banner.
                Rectangle {
                    Layout.fillWidth: true
                    Layout.margins: Theme.spacingM
                    Layout.bottomMargin: 0
                    visible: root.d.showsWarning === true
                    radius: Theme.radiusM
                    color: Qt.rgba(0.75, 0.15, 0.15, 0.14)
                    border.color: Theme.error
                    implicitHeight: warnLabel.implicitHeight + Theme.spacingM
                    Label {
                        id: warnLabel
                        anchors.fill: parent
                        anchors.margins: Theme.spacingS
                        text: "⚠ " + qsTr("Внимание: ") + (root.d.zoneDescription || "")
                        color: Theme.error
                        wrapMode: Text.WordWrap
                        font.pixelSize: 13
                    }
                }

                // Rating + population summary.
                GridLayout {
                    Layout.fillWidth: true
                    Layout.margins: Theme.spacingM
                    Layout.bottomMargin: 0
                    columns: 2
                    columnSpacing: Theme.spacingM
                    rowSpacing: Theme.spacingS

                    StatTile { title: qsTr("Рейтинг"); value: root.d.rating || "—" }
                    StatTile { title: qsTr("Население"); value: root.d.population || "—" }
                }

                // Indicator bars.
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.margins: Theme.spacingM
                    Layout.topMargin: 0
                    spacing: Theme.spacingS

                    Label {
                        text: qsTr("Показатели")
                        color: Theme.onSurface
                        font.pixelSize: 15
                        font.bold: true
                    }
                    IndicatorRow { label: qsTr("Безопасность"); value: root.d.indicators ? root.d.indicators.safety : "" }
                    IndicatorRow { label: qsTr("Экология"); value: root.d.indicators ? root.d.indicators.ecology : "" }
                    IndicatorRow { label: qsTr("Инфраструктура"); value: root.d.indicators ? root.d.indicators.infrastructure : "" }
                    IndicatorRow { label: qsTr("Уровень сервиса"); value: root.d.indicators ? root.d.indicators.service : "" }
                    IndicatorRow { label: qsTr("Бизнес-потенциал"); value: root.d.indicators ? root.d.indicators.businessPotential : "" }
                    IndicatorRow { label: qsTr("Уровень жизни"); value: root.d.indicators ? root.d.indicators.wealth : "" }
                }

                // Recommendations.
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.margins: Theme.spacingM
                    Layout.topMargin: 0
                    Layout.bottomMargin: Theme.spacingL
                    spacing: Theme.spacingXs

                    Label {
                        text: qsTr("Рекомендации")
                        color: Theme.onSurface
                        font.pixelSize: 15
                        font.bold: true
                    }
                    Repeater {
                        model: root.d.recommendations || []
                        delegate: Label {
                            Layout.fillWidth: true
                            text: "• " + modelData
                            color: Theme.onSurfaceVariant
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }
    }
}
