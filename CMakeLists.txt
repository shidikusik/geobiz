# SPDX-License-Identifier: MIT
#
# GeoBiz Uzbekistan — top-level build.
#
# Targets:
#   geobiz_core   — pure C++23 domain library (no Qt): models, classifier,
#                   repository and the local AI engine. Unit-testable anywhere.
#   geobiz        — the Qt Quick application (links geobiz_core + Qt).
#
# Platforms: Windows (.exe + NSIS installer), Linux (AppImage / Flatpak),
# Android (APK / AAB). See the platform folders and docs/BUILD.md.

cmake_minimum_required(VERSION 3.21)

project(GeoBizUzbekistan
    VERSION 1.0.0
    DESCRIPTION "GeoBiz Uzbekistan — district wellbeing, safety, ecology and business potential on Google Maps"
    HOMEPAGE_URL "https://geobiz.uz"
    LANGUAGES CXX)

# --- Global C++ settings -----------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(GEOBIZ_BUILD_TESTS "Build unit tests for the core library" ON)
option(GEOBIZ_CORE_ONLY "Build only the pure core library + tests (no Qt app)" OFF)

# --- Pure core library (no Qt dependency) ------------------------------------
add_library(geobiz_core STATIC
    src/core/District.cpp
    src/core/Classifier.cpp
    src/ai/QueryEngine.cpp
    src/data/DistrictRepository.cpp
)
target_include_directories(geobiz_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(geobiz_core PUBLIC cxx_std_23)
set_target_properties(geobiz_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# --- Fast path: core + tests only (used by the lightweight CI gate) ----------
if(GEOBIZ_CORE_ONLY)
    if(GEOBIZ_BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
    return()
endif()

# --- The Qt application ------------------------------------------------------
# Web backend differs by platform: desktop uses QtWebEngine (Chromium) + a
# QWebChannel bridge; Android/iOS use the native web view via QtWebView, since
# QtWebEngine is not available there.
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core Gui Qml Quick QuickControls2 Concurrent Svg LinguistTools)

if(ANDROID OR IOS)
    find_package(Qt6 6.5 REQUIRED COMPONENTS WebView)
    set(GEOBIZ_WEB_LIBS Qt6::WebView)
else()
    find_package(Qt6 6.5 REQUIRED COMPONENTS WebEngineQuick WebChannel)
    set(GEOBIZ_WEB_LIBS Qt6::WebEngineQuick Qt6::WebChannel)
endif()

qt_standard_project_setup(REQUIRES 6.5)

# Keep generated files tidy. AUTORCC compiles the fixed-path resources.qrc.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_add_executable(geobiz
    src/main.cpp
    src/app/AppConfig.cpp
    src/data/LocalDatasetProvider.cpp
    src/map/MapBridge.cpp
    src/map/DistrictSerializer.cpp
    src/ui/DistrictListModel.cpp
    src/ui/AppController.cpp
    resources.qrc
)

target_link_libraries(geobiz PRIVATE
    geobiz_core
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2
    Qt6::Concurrent Qt6::Svg
    ${GEOBIZ_WEB_LIBS})

# --- QML module --------------------------------------------------------------
# The Theme singleton must be flagged so it is registered as a QML singleton.
set_source_files_properties(qml/theme/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(geobiz
    URI GeoBiz
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/theme/Theme.qml
        qml/components/MapView.qml
        qml/components/MapWebDesktop.qml
        qml/components/MapWebMobile.qml
        qml/components/ZoneLegend.qml
        qml/components/StatTile.qml
        qml/components/IndicatorRow.qml
        qml/components/ApiKeyDialog.qml
        qml/panels/BrowsePanel.qml
        qml/panels/DistrictInfoPanel.qml
        qml/panels/AiAssistantPanel.qml
)

# The web page, dataset and icons are compiled from resources.qrc (added to the
# executable sources above) so they keep stable, code-referenced qrc paths.

# --- Translations ------------------------------------------------------------
qt_add_translations(geobiz
    TS_FILES
        translations/geobiz_ru.ts
        translations/geobiz_uz.ts
        translations/geobiz_en.ts
    RESOURCE_PREFIX "/i18n"
)

# --- Application metadata / packaging hints ----------------------------------
set_target_properties(geobiz PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE OFF
    OUTPUT_NAME "GeoBizUzbekistan"
)

# Android packaging: point Qt at our manifest/gradle template and SDK levels.
if(ANDROID)
    set_target_properties(geobiz PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_MIN_SDK_VERSION 24
        QT_ANDROID_TARGET_SDK_VERSION 34
        QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}"
        QT_ANDROID_VERSION_CODE 1
    )
endif()

# --- Install rules (used by CPack / AppImage / Flatpak) ----------------------
include(GNUInstallDirs)
install(TARGETS geobiz
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(UNIX AND NOT ANDROID AND NOT APPLE)
    install(FILES flatpak/uz.geobiz.GeoBiz.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications OPTIONAL)
    install(FILES flatpak/uz.geobiz.GeoBiz.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo OPTIONAL)
    install(FILES assets/icons/app.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME uz.geobiz.GeoBiz.svg OPTIONAL)
endif()

# --- Tests -------------------------------------------------------------------
if(GEOBIZ_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# --- CPack (Windows installer + generic archives) ----------------------------
include(cmake/Packaging.cmake)
