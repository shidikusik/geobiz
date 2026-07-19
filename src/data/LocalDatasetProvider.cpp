// SPDX-License-Identifier: MIT
#include "geobiz/data/LocalDatasetProvider.hpp"

#include <QByteArray>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <utility>

namespace geobiz::data {

namespace {

core::Metric parseMetric(const QJsonObject& obj, const QString& key) {
    if (!obj.contains(key) || !obj.value(key).isObject()) {
        return core::Metric::missing();
    }
    const QJsonObject m = obj.value(key).toObject();
    if (!m.contains("value") || m.value("value").isNull()) {
        return core::Metric::missing();
    }
    const double value = m.value("value").toDouble();
    const std::string source = m.value("source").toString().toStdString();
    return core::Metric{value, source};
}

core::LatLng parseLatLng(const QJsonObject& obj) {
    return core::LatLng{obj.value("lat").toDouble(), obj.value("lng").toDouble()};
}

core::Polygon parseBoundary(const QJsonValue& value) {
    core::Polygon ring;
    if (!value.isArray()) return ring;
    const QJsonArray arr = value.toArray();
    ring.reserve(static_cast<std::size_t>(arr.size()));
    for (const QJsonValue& v : arr) {
        if (v.isArray()) {
            const QJsonArray pair = v.toArray();
            if (pair.size() >= 2) {
                ring.push_back({pair.at(0).toDouble(), pair.at(1).toDouble()});
            }
        } else if (v.isObject()) {
            ring.push_back(parseLatLng(v.toObject()));
        }
    }
    return ring;
}

core::District parseDistrict(const QJsonObject& obj) {
    core::District d;
    d.id = obj.value("id").toString().toStdString();
    d.parentId = obj.value("parentId").toString().toStdString();

    const QJsonObject nameObj = obj.value("name").toObject();
    d.nameRu = nameObj.value("ru").toString().toStdString();
    d.nameUz = nameObj.value("uz").toString().toStdString();
    d.nameEn = nameObj.value("en").toString().toStdString();

    if (const auto level =
            core::adminLevelFromId(obj.value("level").toString().toStdString())) {
        d.level = *level;
    }

    d.center = parseLatLng(obj.value("center").toObject());
    d.boundary = parseBoundary(obj.value("boundary"));

    // Population is a raw count, kept unclamped in its own optional field.
    if (obj.contains("population") && obj.value("population").isObject()) {
        const QJsonObject pop = obj.value("population").toObject();
        if (pop.contains("value") && !pop.value("value").isNull()) {
            d.populationCount =
                static_cast<std::int64_t>(pop.value("value").toDouble());
            d.populationSource = pop.value("source").toString().toStdString();
        }
    }

    const QJsonObject ind = obj.value("indicators").toObject();
    d.indicators.safety = parseMetric(ind, "safety");
    d.indicators.ecology = parseMetric(ind, "ecology");
    d.indicators.infrastructure = parseMetric(ind, "infrastructure");
    d.indicators.service = parseMetric(ind, "service");
    d.indicators.businessPotential = parseMetric(ind, "businessPotential");
    d.indicators.wealth = parseMetric(ind, "wealth");
    d.indicators.healthHazard = parseMetric(ind, "healthHazard");
    d.indicators.constructionRisk = parseMetric(ind, "constructionRisk");
    d.indicators.technogenicRisk = parseMetric(ind, "technogenicRisk");
    d.indicators.crime = parseMetric(ind, "crime");

    for (const QJsonValue& r : obj.value("recommendations").toArray()) {
        d.recommendations.push_back(r.toString().toStdString());
    }
    return d;
}

}  // namespace

LocalDatasetProvider::LocalDatasetProvider(QString path) : path_(std::move(path)) {}

std::string LocalDatasetProvider::name() const { return "local-dataset"; }

DatasetResult LocalDatasetProvider::load() {
    DatasetResult result;
    result.sourceName = "local-dataset";

    QFile file(path_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.ok = false;
        result.message = QString("Не удалось открыть %1").arg(path_).toStdString();
        return result;
    }

    const QByteArray bytes = file.readAll();
    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        result.ok = false;
        result.message =
            QString("Ошибка разбора JSON: %1").arg(err.errorString()).toStdString();
        return result;
    }

    const QJsonObject root = doc.object();
    result.sourceName = root.value("source").toString("local-dataset").toStdString();

    const QJsonArray arr = root.value("districts").toArray();
    result.districts.reserve(static_cast<std::size_t>(arr.size()));
    for (const QJsonValue& v : arr) {
        if (v.isObject()) {
            result.districts.push_back(parseDistrict(v.toObject()));
        }
    }

    result.ok = true;
    result.message = QString("Загружено территорий: %1")
                         .arg(result.districts.size())
                         .toStdString();
    return result;
}

}  // namespace geobiz::data
