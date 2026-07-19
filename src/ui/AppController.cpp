// SPDX-License-Identifier: MIT
#include "geobiz/ui/AppController.hpp"

#include <QDir>
#include <QFile>
#include <QLocale>
#include <QStandardPaths>
#include <QtConcurrent/QtConcurrent>

#include "geobiz/ai/QueryEngine.hpp"
#include "geobiz/core/Zone.hpp"
#include "geobiz/data/LocalDatasetProvider.hpp"
#include "geobiz/map/DistrictSerializer.hpp"

namespace geobiz::ui {

namespace {
constexpr const char* kBundledDataset = ":/data/uzbekistan.json";
}  // namespace

AppController::AppController(QObject* parent)
    : QObject(parent),
      repository_(std::make_unique<data::DistrictRepository>()),
      mapBridge_(std::make_unique<map::MapBridge>()),
      searchModel_(std::make_unique<DistrictListModel>()),
      aiModel_(std::make_unique<DistrictListModel>()) {
    // Resolve configuration (API key, optional external dataset path).
    config_ = app::AppConfig::load();

    // The bundled dataset is the source of truth; additional enrichment
    // providers (Google Places, open-data feeds) can be registered here later.
    const QString datasetPath = config_.datasetPath().isEmpty()
                                    ? QString(kBundledDataset)
                                    : config_.datasetPath();
    repository_->addProvider(
        std::make_shared<data::LocalDatasetProvider>(datasetPath));

    mapBridge_->setApiKey(config_.googleMapsApiKey());

    connect(mapBridge_.get(), &map::MapBridge::districtSelected, this,
            &AppController::onMapDistrictSelected);
    connect(mapBridge_.get(), &map::MapBridge::mapErrorOccurred, this,
            [this](const QString& msg) { emit errorOccurred(msg); });

    connect(&loadWatcher_, &QFutureWatcher<QString>::finished, this,
            &AppController::onLoadFinished);
}

AppController::~AppController() {
    // Ensure any in-flight load completes before the repository is destroyed.
    if (loadWatcher_.isRunning()) {
        loadWatcher_.waitForFinished();
    }
}

void AppController::setLoading(bool value) {
    if (loading_ != value) {
        loading_ = value;
        emit loadingChanged();
    }
}

void AppController::setStatus(const QString& message) {
    if (status_ != message) {
        status_ = message;
        emit statusChanged();
    }
}

void AppController::start() { reload(); }

void AppController::reload() {
    if (loading_) return;
    setLoading(true);
    setStatus(QStringLiteral("Загрузка данных…"));

    // Reload + classify + serialise on a worker thread. The lambda touches the
    // repository exclusively until it returns, and the UI does not read it while
    // `ready_` is false, so no locking is required.
    auto future = QtConcurrent::run([this]() -> QString {
        repository_->reload();
        return map::buildMapPayload(repository_->districts());
    });
    loadWatcher_.setFuture(future);
}

void AppController::onLoadFinished() {
    const QString payload = loadWatcher_.result();
    mapBridge_->setDistrictPayload(payload);

    ready_ = true;
    emit readyChanged();
    setLoading(false);

    const auto count = repository_->districts().size();
    setStatus(QStringLiteral("Готово. Территорий загружено: %1").arg(count));

    showTopLevel();
}

void AppController::onMapDistrictSelected(const QString& id) {
    selectDistrict(id);
}

QVariantList AppController::zoneLegend() const {
    QVariantList list;
    for (const core::ZoneInfo& zi : core::allZones()) {
        if (zi.type == core::ZoneType::Unknown) continue;  // legend skips "no data"
        QVariantMap m;
        m["id"] = QString::fromStdString(std::string(zi.id));
        m["name"] = QString::fromStdString(std::string(zi.nameRu));
        m["color"] = QString::fromStdString(std::string(zi.cssColor));
        m["description"] = QString::fromStdString(std::string(zi.descriptionRu));
        m["showsWarning"] = zi.showsWarning;
        list.append(m);
    }
    return list;
}

void AppController::search(const QString& query) {
    if (!ready_) return;
    std::vector<DistrictListModel::Row> rows;
    if (query.trimmed().isEmpty()) {
        showTopLevel();
        return;
    }
    for (const core::District* d :
         repository_->search(query.toStdString(), 30)) {
        rows.push_back({d, {}, -1.0});
    }
    searchModel_->setRows(std::move(rows));
}

void AppController::showTopLevel() {
    if (!ready_) return;
    std::vector<DistrictListModel::Row> rows;
    for (const core::District& d : repository_->districts()) {
        if (d.level == core::AdminLevel::Region ||
            d.level == core::AdminLevel::City) {
            rows.push_back({&d, {}, -1.0});
        }
    }
    searchModel_->setRows(std::move(rows));
}

QUrl AppController::mapPageUrl() {
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    // QtWebView's native backend cannot read Qt resources, so copy the bundled
    // web assets to a real directory once and serve them over file://.
    const QString dir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
        "/web";
    QDir().mkpath(dir);
    const auto copyOut = [&](const QString& res, const QString& name) {
        const QString dest = dir + "/" + name;
        QFile::remove(dest);  // always refresh to the bundled version
        QFile::copy(res, dest);
        QFile::setPermissions(dest, QFile::ReadOwner | QFile::WriteOwner |
                                        QFile::ReadUser | QFile::ReadOther);
    };
    copyOut(QStringLiteral(":/web/map.html"), QStringLiteral("map.html"));
    copyOut(QStringLiteral(":/web/map.js"), QStringLiteral("map.js"));
    return QUrl::fromLocalFile(dir + "/map.html");
#else
    return QUrl(QStringLiteral("qrc:/web/map.html"));
#endif
}

QVariantMap AppController::toVariantMap(const core::District& d) const {
    QVariantMap m;
    const core::ZoneInfo& zi = core::zoneInfo(d.zone);
    const QString noData = QStringLiteral("Недостаточно данных");

    m["id"] = QString::fromStdString(d.id);
    m["name"] = QString::fromStdString(d.nameRu);
    m["nameUz"] = QString::fromStdString(d.nameUz);
    m["nameEn"] = QString::fromStdString(d.nameEn);
    m["level"] = QString::fromStdString(std::string(core::adminLevelNameRu(d.level)));
    m["lat"] = d.center.lat;
    m["lng"] = d.center.lng;

    m["hasData"] = d.hasSufficientData;
    m["zoneId"] = QString::fromStdString(std::string(zi.id));
    m["zoneName"] = d.hasSufficientData
                        ? QString::fromStdString(std::string(zi.nameRu))
                        : noData;
    m["zoneColor"] = QString::fromStdString(std::string(zi.cssColor));
    m["zoneDescription"] =
        QString::fromStdString(std::string(zi.descriptionRu));
    m["showsWarning"] = d.hasSufficientData && zi.showsWarning;
    m["rating"] = d.hasSufficientData ? QString::number(d.rating, 'f', 1) : noData;

    // Population — formatted with thousands separators, or the honest fallback.
    if (d.populationCount.has_value()) {
        m["population"] = QLocale().toString(
            static_cast<qlonglong>(*d.populationCount));
        m["populationSource"] = QString::fromStdString(d.populationSource);
    } else {
        m["population"] = noData;
        m["populationSource"] = QString();
    }

    // Each indicator: numeric string when present, "Недостаточно данных" else.
    auto metric = [&](const core::Metric& mtr) -> QString {
        return mtr.hasValue() ? QString::number(mtr.value(), 'f', 0) : noData;
    };
    QVariantMap ind;
    ind["safety"] = metric(d.indicators.safety);
    ind["ecology"] = metric(d.indicators.ecology);
    ind["infrastructure"] = metric(d.indicators.infrastructure);
    ind["service"] = metric(d.indicators.service);
    ind["businessPotential"] = metric(d.indicators.businessPotential);
    ind["wealth"] = metric(d.indicators.wealth);
    m["indicators"] = ind;

    QVariantList recs;
    for (const std::string& r : d.recommendations) {
        recs.append(QString::fromStdString(r));
    }
    m["recommendations"] = recs;

    return m;
}

void AppController::selectDistrict(const QString& id) {
    if (!ready_) return;
    const core::District* d = repository_->find(id.toStdString());
    if (d == nullptr) return;

    selected_ = toVariantMap(*d);
    emit selectedDistrictChanged();

    // Fly the map to the district and highlight it.
    const int zoom = d->level == core::AdminLevel::Region ? 9 : 12;
    mapBridge_->focusOn(d->center.lat, d->center.lng, zoom);
    mapBridge_->highlight(id);
}

void AppController::clearSelection() {
    if (selected_.isEmpty()) return;
    selected_.clear();
    emit selectedDistrictChanged();
}

QString AppController::askAi(const QString& query) {
    if (!ready_) {
        aiSummary_ = QStringLiteral("Данные ещё загружаются…");
        emit aiSummaryChanged();
        return aiSummary_;
    }

    ai::QueryEngine engine(repository_->districts());
    const ai::Answer answer = engine.ask(query.toStdString(), 6);

    std::vector<DistrictListModel::Row> rows;
    for (const ai::Suggestion& s : answer.suggestions) {
        if (const core::District* d = repository_->find(s.districtId)) {
            rows.push_back(
                {d, QString::fromStdString(s.rationaleRu), s.score});
        }
    }
    aiModel_->setRows(std::move(rows));

    aiSummary_ = QString::fromStdString(answer.summaryRu);
    emit aiSummaryChanged();
    return aiSummary_;
}

void AppController::setApiKey(const QString& key) {
    if (config_.saveGoogleMapsApiKey(key)) {
        mapBridge_->setApiKey(config_.googleMapsApiKey());
        emit apiKeyChanged();
        setStatus(QStringLiteral("Ключ API сохранён. Перезагрузите карту."));
    } else {
        emit errorOccurred(QStringLiteral("Не удалось сохранить ключ API."));
    }
}

}  // namespace geobiz::ui
