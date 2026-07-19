// SPDX-License-Identifier: MIT
//
// AppController.hpp — the single façade QML talks to.
//
// It owns the configuration, the district repository, the map bridge, the local
// AI engine and the list models, and exposes them to QML through properties and
// invokable methods. Data loading and (re)classification run on a worker thread
// so the UI never blocks, in line with the performance requirements.

#pragma once

#include <memory>

#include <QFutureWatcher>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>

#include "geobiz/app/AppConfig.hpp"
#include "geobiz/data/DistrictRepository.hpp"
#include "geobiz/map/MapBridge.hpp"
#include "geobiz/ui/DistrictListModel.hpp"

namespace geobiz::ui {

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(bool hasApiKey READ hasApiKey NOTIFY apiKeyChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusChanged)
    Q_PROPERTY(QObject* mapBridge READ mapBridge CONSTANT)
    Q_PROPERTY(QObject* searchResults READ searchResults CONSTANT)
    Q_PROPERTY(QObject* aiResults READ aiResults CONSTANT)
    Q_PROPERTY(QVariantMap selectedDistrict READ selectedDistrict NOTIFY
                   selectedDistrictChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectedDistrictChanged)
    Q_PROPERTY(QVariantList zoneLegend READ zoneLegend CONSTANT)
    Q_PROPERTY(QString aiSummary READ aiSummary NOTIFY aiSummaryChanged)

public:
    explicit AppController(QObject* parent = nullptr);
    ~AppController() override;

    [[nodiscard]] bool loading() const { return loading_; }
    [[nodiscard]] bool ready() const { return ready_; }
    [[nodiscard]] bool hasApiKey() const { return config_.hasGoogleMapsApiKey(); }
    [[nodiscard]] QString statusMessage() const { return status_; }
    [[nodiscard]] QObject* mapBridge() const { return mapBridge_.get(); }
    [[nodiscard]] QObject* searchResults() const { return searchModel_.get(); }
    [[nodiscard]] QObject* aiResults() const { return aiModel_.get(); }
    [[nodiscard]] QVariantMap selectedDistrict() const { return selected_; }
    [[nodiscard]] bool hasSelection() const { return !selected_.isEmpty(); }
    [[nodiscard]] QVariantList zoneLegend() const;
    [[nodiscard]] QString aiSummary() const { return aiSummary_; }

    /// Kicks off the asynchronous initial load. Safe to call once at startup.
    Q_INVOKABLE void start();

    /// Reloads all data from the providers (async).
    Q_INVOKABLE void reload();

    /// Filters the search-results model by the given query.
    Q_INVOKABLE void search(const QString& query);

    /// Selects a district by id and updates `selectedDistrict`. Also tells the
    /// map to fly to and highlight it.
    Q_INVOKABLE void selectDistrict(const QString& id);
    Q_INVOKABLE void clearSelection();

    /// Runs the local AI engine; fills `aiResults` and returns the summary text.
    Q_INVOKABLE QString askAi(const QString& query);

    /// Persists a Google Maps API key entered by the user and re-inits the map.
    Q_INVOKABLE void setApiKey(const QString& key);

    /// Convenience for QML: the top-level regions to show in the browse panel.
    Q_INVOKABLE void showTopLevel();

    /// Returns the URL of the map page for the current platform. On desktop this
    /// is the qrc page loaded by QtWebEngine directly; on mobile (QtWebView,
    /// whose native web view cannot read qrc) the bundled web assets are first
    /// extracted to the app data directory and a file:// URL is returned.
    Q_INVOKABLE QUrl mapPageUrl();

signals:
    void loadingChanged();
    void readyChanged();
    void apiKeyChanged();
    void statusChanged();
    void selectedDistrictChanged();
    void aiSummaryChanged();
    void errorOccurred(const QString& message);

private slots:
    void onLoadFinished();
    void onMapDistrictSelected(const QString& id);

private:
    void setLoading(bool value);
    void setStatus(const QString& message);
    [[nodiscard]] QVariantMap toVariantMap(const core::District& d) const;

    app::AppConfig                              config_;
    std::unique_ptr<data::DistrictRepository>   repository_;
    std::unique_ptr<map::MapBridge>             mapBridge_;
    std::unique_ptr<DistrictListModel>          searchModel_;
    std::unique_ptr<DistrictListModel>          aiModel_;

    QFutureWatcher<QString>                     loadWatcher_;  ///< Yields payload.

    QVariantMap selected_;
    QString     status_;
    QString     aiSummary_;
    bool        loading_ = false;
    bool        ready_ = false;
};

}  // namespace geobiz::ui
