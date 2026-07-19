// SPDX-License-Identifier: MIT
// AiAssistantPanel.qml — the local AI assistant UI.
// Sends the user's Russian question to App.askAi() and lists ranked districts
// from App.aiResults. Includes example prompts. Emits suggestionActivated(id).
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import GeoBiz

Item {
    id: root
    signal suggestionActivated(string id)

    readonly property var examples: [
        qsTr("Где лучше открыть кофейню?"),
        qsTr("Где меньше конкуренция?"),
        qsTr("Самые богатые районы Ташкента"),
        qsTr("Лучшие районы для ресторана"),
        qsTr("Самые безопасные районы")
    ]

    function submit(text) {
        if (text && text.trim().length > 0) {
            queryField.text = text
            App.askAi(text)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingM
        spacing: Theme.spacingS

        Label {
            text: qsTr("ИИ-помощник")
            color: Theme.onSurface
            font.pixelSize: 18
            font.bold: true
        }
        Label {
            Layout.fillWidth: true
            text: qsTr("Локальный помощник отвечает на основе загруженных данных. " +
                       "Если данных недостаточно, он честно сообщит об этом.")
            color: Theme.onSurfaceVariant
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingS
            TextField {
                id: queryField
                Layout.fillWidth: true
                placeholderText: qsTr("Задайте вопрос…")
                Material.accent: Theme.primary
                onAccepted: root.submit(text)
            }
            Button {
                text: qsTr("Спросить")
                Material.background: Theme.primary
                Material.foreground: Theme.onPrimary
                onClicked: root.submit(queryField.text)
            }
        }

        // Example chips.
        Flow {
            Layout.fillWidth: true
            spacing: Theme.spacingXs
            Repeater {
                model: root.examples
                delegate: Button {
                    text: modelData
                    flat: true
                    font.pixelSize: 11
                    Material.foreground: Theme.primary
                    onClicked: root.submit(modelData)
                }
            }
        }

        // Answer summary.
        Label {
            Layout.fillWidth: true
            visible: App.aiSummary.length > 0
            text: App.aiSummary
            color: Theme.onSurface
            font.pixelSize: 14
            font.bold: true
            wrapMode: Text.WordWrap
            topPadding: Theme.spacingS
        }

        // Ranked suggestions.
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: Theme.spacingXs
            model: App.aiResults
            ScrollBar.vertical: ScrollBar {}

            delegate: ItemDelegate {
                width: ListView.view.width
                height: 70
                onClicked: root.suggestionActivated(model.districtId)
                contentItem: RowLayout {
                    spacing: Theme.spacingM
                    Rectangle {
                        width: 12; height: 12; radius: 6
                        color: model.hasData ? model.zoneColor : Theme.outline
                        Layout.alignment: Qt.AlignVCenter
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Label {
                            text: model.name
                            color: Theme.onSurface
                            font.pixelSize: 15
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Label {
                            text: model.rationale
                            color: Theme.onSurfaceVariant
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                    Label {
                        text: Number(model.rating).toFixed(0)
                        color: Theme.primary
                        font.pixelSize: 16
                        font.bold: true
                    }
                }
            }
        }
    }
}
