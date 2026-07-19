// SPDX-License-Identifier: MIT
// BrowsePanel.qml — search field + a list of territories (search results, or the
// top-level regions when the query is empty). Emits districtActivated(id).
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import GeoBiz

Item {
    id: root
    signal districtActivated(string id)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingM
        spacing: Theme.spacingS

        Label {
            text: qsTr("Территории Узбекистана")
            color: Theme.onSurface
            font.pixelSize: 16
            font.bold: true
        }

        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: qsTr("Поиск области, города, района…")
            leftPadding: Theme.spacingM
            Material.accent: Theme.primary
            onTextChanged: searchDebounce.restart()
        }
        Timer {
            id: searchDebounce
            interval: 180
            onTriggered: App.search(searchField.text)
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: Theme.spacingXs
            model: App.searchResults
            ScrollBar.vertical: ScrollBar {}

            delegate: ItemDelegate {
                width: ListView.view.width
                height: 64
                onClicked: root.districtActivated(model.districtId)

                contentItem: RowLayout {
                    spacing: Theme.spacingM
                    // Zone colour swatch (neutral when data is insufficient).
                    Rectangle {
                        Layout.alignment: Qt.AlignVCenter
                        width: 14; height: 14; radius: 7
                        color: model.hasData ? model.zoneColor : "transparent"
                        border.width: model.hasData ? 0 : 1
                        border.color: Theme.outline
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
                            text: model.hasData
                                  ? model.levelName + " · " + model.zoneName
                                  : model.levelName + " · " + qsTr("Недостаточно данных")
                            color: Theme.onSurfaceVariant
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                    Label {
                        visible: model.hasData
                        text: Number(model.rating).toFixed(0)
                        color: Theme.primary
                        font.pixelSize: 15
                        font.bold: true
                    }
                }
            }

            // Empty-state hint.
            Label {
                anchors.centerIn: parent
                width: parent.width - Theme.spacingL
                visible: list.count === 0
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: App.ready ? qsTr("Ничего не найдено")
                                : qsTr("Загрузка данных…")
                color: Theme.onSurfaceVariant
            }
        }
    }
}
