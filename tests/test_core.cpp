// SPDX-License-Identifier: MIT
//
// test_core.cpp — lightweight, dependency-free tests for the pure core library.
//
// Covers the two most important invariants of the project:
//   * classification never fabricates a zone when data is insufficient, and
//   * the local AI engine ranks correctly and admits when it cannot answer.

#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

#include "geobiz/ai/QueryEngine.hpp"
#include "geobiz/core/Classifier.hpp"
#include "geobiz/core/Zone.hpp"

using namespace geobiz;

namespace {

int g_failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            std::printf("FAIL: %s (line %d)\n", #cond, __LINE__);          \
            ++g_failures;                                                  \
        }                                                                  \
    } while (0)

core::District makeDistrict(const std::string& id, const std::string& name) {
    core::District d;
    d.id = id;
    d.nameRu = name;
    d.level = core::AdminLevel::District;
    return d;
}

void testInsufficientDataStaysUnknown() {
    core::District d = makeDistrict("empty", "Пустой");
    core::applyClassification(d);
    CHECK(d.zone == core::ZoneType::Unknown);
    CHECK(d.hasSufficientData == false);

    // One indicator is still below the minimum threshold of three.
    d.indicators.safety = core::Metric{90, "test"};
    core::applyClassification(d);
    CHECK(d.zone == core::ZoneType::Unknown);
}

void testGreenClassification() {
    core::District d = makeDistrict("green", "Зелёный");
    d.indicators.safety = core::Metric{85, "t"};
    d.indicators.ecology = core::Metric{80, "t"};
    d.indicators.infrastructure = core::Metric{78, "t"};
    d.indicators.service = core::Metric{70, "t"};
    d.indicators.businessPotential = core::Metric{72, "t"};
    d.indicators.wealth = core::Metric{82, "t"};
    core::applyClassification(d);
    CHECK(d.hasSufficientData);
    CHECK(d.zone == core::ZoneType::Green);
    CHECK(d.rating >= 72.0);
}

void testHazardPriorityOverQuality() {
    // Wealthy AND safe but with high technogenic risk must be RED.
    core::District d = makeDistrict("risky", "Рисковый");
    d.indicators.safety = core::Metric{90, "t"};
    d.indicators.wealth = core::Metric{90, "t"};
    d.indicators.infrastructure = core::Metric{90, "t"};
    d.indicators.technogenicRisk = core::Metric{80, "t"};
    core::applyClassification(d);
    CHECK(d.zone == core::ZoneType::Red);
}

void testZoneTableConsistency() {
    CHECK(core::zoneInfo(core::ZoneType::Gray).showsWarning);
    CHECK(core::zoneInfo(core::ZoneType::Green).showsWarning == false);
    CHECK(core::zoneFromId("purple") == core::ZoneType::Purple);
    CHECK(core::zoneFromId("nonexistent") == core::ZoneType::Unknown);
}

void testAiRanksSafest() {
    std::vector<core::District> districts;
    core::District a = makeDistrict("a", "Альфа");
    a.indicators.safety = core::Metric{40, "t"};
    core::District b = makeDistrict("b", "Бета");
    b.indicators.safety = core::Metric{90, "t"};
    districts.push_back(a);
    districts.push_back(b);

    ai::QueryEngine engine(districts);
    ai::Answer ans = engine.ask("Самые безопасные районы");
    CHECK(ans.sufficientData);
    CHECK(ans.suggestions.size() == 2);
    CHECK(ans.suggestions.front().districtId == "b");  // safest first
}

void testAiAdmitsInsufficientData() {
    std::vector<core::District> districts;
    districts.push_back(makeDistrict("x", "Икс"));  // no indicators at all

    ai::QueryEngine engine(districts);
    ai::Answer ans = engine.ask("Самые богатые районы");
    CHECK(ans.sufficientData == false);
    CHECK(ans.suggestions.empty());
}

void testAiScopeFilter() {
    std::vector<core::District> districts;
    core::District t = makeDistrict("uz.tashkent-city.yunusabad", "Юнусабад");
    t.parentId = "uz.tashkent-city";
    t.indicators.wealth = core::Metric{80, "t"};
    core::District s = makeDistrict("uz.samarkand", "Самаркандская область");
    s.indicators.wealth = core::Metric{95, "t"};
    districts.push_back(t);
    districts.push_back(s);

    ai::QueryEngine engine(districts);
    ai::Answer ans = engine.ask("Самые богатые районы Ташкента");
    CHECK(ans.sufficientData);
    // Only the Tashkent district should be in scope despite Samarkand scoring
    // higher — scope filtering must exclude out-of-scope territories.
    CHECK(ans.suggestions.size() == 1);
    CHECK(ans.suggestions.front().districtId == "uz.tashkent-city.yunusabad");
}

}  // namespace

int main() {
    testInsufficientDataStaysUnknown();
    testGreenClassification();
    testHazardPriorityOverQuality();
    testZoneTableConsistency();
    testAiRanksSafest();
    testAiAdmitsInsufficientData();
    testAiScopeFilter();

    if (g_failures == 0) {
        std::printf("All core tests passed.\n");
        return 0;
    }
    std::printf("%d test(s) failed.\n", g_failures);
    return 1;
}
