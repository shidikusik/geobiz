// SPDX-License-Identifier: MIT
//
// main.cpp — GeoBiz Uzbekistan application entry point.
//
// Boots a Qt Quick (QML) application using the Material Design style, wires the
// native AppController façade into the QML context, installs translations, and
// shows the main window. All heavy lifting (data, maps, AI) lives behind
// AppController; main() stays intentionally thin.

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QTranslator>

// The web backend differs per platform (see CMakeLists.txt): QtWebEngine on
// desktop, QtWebView on mobile. Each has its own one-time initialisation.
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
#  include <QtWebView>
#else
#  include <QtWebEngineQuick>
#endif

#include "geobiz/ui/AppController.hpp"

int main(int argc, char* argv[]) {
    // The web backend must be initialised before the QGuiApplication is created.
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    QtWebView::initialize();
#else
    QtWebEngineQuick::initialize();
#endif

    QGuiApplication app(argc, argv);

    // Identity used by QSettings / QStandardPaths for the config file location.
    QCoreApplication::setOrganizationName(QStringLiteral("GeoBiz"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("geobiz.uz"));
    QCoreApplication::setApplicationName(QStringLiteral("GeoBiz Uzbekistan"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/assets/icons/app.svg")));

    // Material Design 3 look. The variant/scheme can be overridden by the user
    // from the settings panel; these are sensible defaults.
    QQuickStyle::setStyle(QStringLiteral("Material"));

    // Install bundled translations for the current UI language. The primary
    // language is Russian; Uzbek and English catalogues are also shipped.
    QTranslator translator;
    const QString locale = QLocale::system().name();  // e.g. "ru_RU"
    if (translator.load(QStringLiteral(":/i18n/geobiz_") + locale.section('_', 0, 0))) {
        QCoreApplication::installTranslator(&translator);
    }

    // Expose the DistrictListModel roles enum to QML for readability.
    qmlRegisterUncreatableType<geobiz::ui::DistrictListModel>(
        "GeoBiz", 1, 0, "DistrictListModel",
        QStringLiteral("Provided by the C++ backend."));

    geobiz::ui::AppController controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("App"), &controller);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.loadFromModule("GeoBiz", "Main");
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    // Begin the asynchronous data load once the UI is up.
    controller.start();

    return app.exec();
}
