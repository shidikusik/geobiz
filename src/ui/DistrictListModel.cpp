// SPDX-License-Identifier: MIT
#include "geobiz/ui/DistrictListModel.hpp"

#include <utility>

#include "geobiz/core/Zone.hpp"

namespace geobiz::ui {

DistrictListModel::DistrictListModel(QObject* parent)
    : QAbstractListModel(parent) {}

int DistrictListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(rows_.size());
}

QVariant DistrictListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 ||
        index.row() >= static_cast<int>(rows_.size())) {
        return {};
    }
    const Row& row = rows_[static_cast<std::size_t>(index.row())];
    const core::District* d = row.district;
    if (d == nullptr) return {};
    const core::ZoneInfo& zi = core::zoneInfo(d->zone);

    switch (role) {
        case IdRole:
            return QString::fromStdString(d->id);
        case NameRole:
            return QString::fromStdString(d->nameRu);
        case LevelRole:
            return QString::fromStdString(
                std::string(core::adminLevelNameRu(d->level)));
        case ZoneIdRole:
            return QString::fromStdString(std::string(zi.id));
        case ZoneNameRole:
            return QString::fromStdString(std::string(zi.nameRu));
        case ZoneColorRole:
            return QString::fromStdString(std::string(zi.cssColor));
        case RatingRole:
            return row.rating >= 0.0 ? row.rating : d->rating;
        case HasDataRole:
            return d->hasSufficientData;
        case RationaleRole:
            return row.rationale;
        case LatRole:
            return d->center.lat;
        case LngRole:
            return d->center.lng;
        default:
            return {};
    }
}

QHash<int, QByteArray> DistrictListModel::roleNames() const {
    return {
        {IdRole, "districtId"},   {NameRole, "name"},
        {LevelRole, "levelName"}, {ZoneIdRole, "zoneId"},
        {ZoneNameRole, "zoneName"}, {ZoneColorRole, "zoneColor"},
        {RatingRole, "rating"},   {HasDataRole, "hasData"},
        {RationaleRole, "rationale"}, {LatRole, "lat"},
        {LngRole, "lng"},
    };
}

void DistrictListModel::setRows(std::vector<Row> rows) {
    beginResetModel();
    rows_ = std::move(rows);
    endResetModel();
}

void DistrictListModel::clear() {
    beginResetModel();
    rows_.clear();
    endResetModel();
}

QString DistrictListModel::idAt(int row) const {
    if (row < 0 || row >= static_cast<int>(rows_.size())) return {};
    const core::District* d = rows_[static_cast<std::size_t>(row)].district;
    return d != nullptr ? QString::fromStdString(d->id) : QString{};
}

}  // namespace geobiz::ui
