// SPDX-License-Identifier: MIT
// ApiKeyDialog.qml — collects a Google Maps API key from the user.
// Emits accepted(key). The key is persisted by the C++ AppConfig.
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import GeoBiz

Dialog {
    id: root
    signal accepted(string key)

    modal: true
    title: qsTr("Ключ Google Maps API")
    width: Math.min(480, parent ? parent.width - Theme.spacingXl : 480)
    standardButtons: Dialog.Ok | Dialog.Cancel
    Material.background: Theme.surface1

    onAccepted: root.accepted(keyField.text.trim())

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spacingM

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Введите ключ Google Maps API. Включите в консоли Google " +
                       "Cloud: Maps JavaScript API, Places API и Geocoding API.")
            color: Theme.onSurfaceVariant
            font.pixelSize: 13
        }
        TextField {
            id: keyField
            Layout.fillWidth: true
            placeholderText: "AIza…"
            Material.accent: Theme.primary
            echoMode: TextInput.Normal
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Ключ хранится локально на вашем устройстве и используется " +
                       "только для загрузки карты.")
            color: Theme.onSurfaceVariant
            font.pixelSize: 11
        }
    }
}
