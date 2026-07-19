// SPDX-License-Identifier: MIT
// Theme.qml — a singleton holding the Material Design 3 palette + spacing scale.
// Registered as a QML singleton so any component can reference `Theme.*` without
// prop-drilling. Colours follow an MD3 tonal scheme with a light/dark split.
pragma Singleton
import QtQuick

QtObject {
    id: theme

    // ---- Dark-mode toggle ---------------------------------------------------
    property bool dark: false

    // ---- MD3 seed / key colours --------------------------------------------
    readonly property color primary:        dark ? "#9ecaff" : "#0061a4"
    readonly property color onPrimary:      dark ? "#003258" : "#ffffff"
    readonly property color primaryContainer: dark ? "#00497d" : "#d1e4ff"
    readonly property color secondary:      dark ? "#bbc7db" : "#535f70"
    readonly property color surface:        dark ? "#111418" : "#fdfcff"
    readonly property color surfaceVariant: dark ? "#1c2126" : "#f0f2f7"
    readonly property color onSurface:      dark ? "#e2e2e6" : "#1a1c1e"
    readonly property color onSurfaceVariant: dark ? "#c3c7cf" : "#42474e"
    readonly property color outline:        dark ? "#8c9199" : "#72777f"
    readonly property color error:          dark ? "#ffb4ab" : "#ba1a1a"
    readonly property color scrim:          "#80000000"

    // Elevation surface tints (MD3 surface containers).
    readonly property color surface1: dark ? "#1b1f24" : "#f4f4f8"
    readonly property color surface2: dark ? "#20252b" : "#eeeef4"
    readonly property color surface3: dark ? "#252b31" : "#e8e9f0"

    // ---- Spacing / radius scale --------------------------------------------
    readonly property int spacingXs: 4
    readonly property int spacingS:  8
    readonly property int spacingM:  16
    readonly property int spacingL:  24
    readonly property int spacingXl: 32

    readonly property int radiusS: 8
    readonly property int radiusM: 12
    readonly property int radiusL: 20
    readonly property int radiusXl: 28

    // Adaptive breakpoint: below this width the layout collapses to a single
    // column with overlay drawers (phones / narrow windows).
    readonly property int compactWidth: 720
}
