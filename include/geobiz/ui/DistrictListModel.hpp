// SPDX-License-Identifier: MIT
//
// DistrictListModel.hpp — a QAbstractListModel exposing districts to QML lists.
//
// Used for both the search-results list and the AI-suggestions list. It holds
// non-owning pointers into the repository's district vector, which is valid for
// the lifetime of the app (the repository outlives the models).

#pragma once

#include <vector>

#include <QAbstractListModel>
#include <QHash>

#include "geobiz/core/District.hpp"

namespace geobiz::ui {

class DistrictListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        LevelRole,
        ZoneIdRole,
        ZoneNameRole,
        ZoneColorRole,
        RatingRole,
        HasDataRole,
        RationaleRole,  ///< Optional per-row explanation (AI suggestions).
        LatRole,
        LngRole,
    };
    Q_ENUM(Roles)

    explicit DistrictListModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(
        const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /// One row's data. `rationale` and `ratingOverride` are optional (used by AI
    /// suggestions where the score/reason differ from the classifier rating).
    struct Row {
        const core::District* district = nullptr;
        QString               rationale;
        double                rating = -1.0;  ///< <0 ⇒ use classifier rating.
    };

    /// Replaces the model contents.
    void setRows(std::vector<Row> rows);
    void clear();

    [[nodiscard]] QString idAt(int row) const;

private:
    std::vector<Row> rows_;
};

}  // namespace geobiz::ui
