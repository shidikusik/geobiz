// SPDX-License-Identifier: MIT
#include "geobiz/core/District.hpp"

#include <array>

namespace geobiz::core {

namespace {
struct LevelInfo {
    AdminLevel       level;
    std::string_view id;
    std::string_view nameRu;
};

constexpr std::array<LevelInfo, 5> kLevels{{
    {AdminLevel::Country, "country", "Страна"},
    {AdminLevel::Region, "region", "Область"},
    {AdminLevel::City, "city", "Город"},
    {AdminLevel::District, "district", "Район"},
    {AdminLevel::Mahalla, "mahalla", "Махалля"},
}};
}  // namespace

std::string_view adminLevelId(AdminLevel level) noexcept {
    for (const auto& l : kLevels) {
        if (l.level == level) return l.id;
    }
    return "district";
}

std::string_view adminLevelNameRu(AdminLevel level) noexcept {
    for (const auto& l : kLevels) {
        if (l.level == level) return l.nameRu;
    }
    return "Район";
}

std::optional<AdminLevel> adminLevelFromId(std::string_view id) noexcept {
    for (const auto& l : kLevels) {
        if (l.id == id) return l.level;
    }
    return std::nullopt;
}

}  // namespace geobiz::core
