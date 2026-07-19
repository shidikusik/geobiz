// SPDX-License-Identifier: MIT
#include "geobiz/core/Classifier.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace geobiz::core {

namespace {

// Collects the present positive indicators together with the weight each one
// contributes to the composite score. Weights sum to 1.0 across the full set;
// when some are missing the present ones are renormalised.
struct WeightedMetric {
    const Metric* metric;
    double        weight;
};

std::vector<WeightedMetric> positiveMetrics(const Indicators& ind) {
    return {
        {&ind.safety, 0.22},
        {&ind.ecology, 0.14},
        {&ind.infrastructure, 0.18},
        {&ind.service, 0.12},
        {&ind.businessPotential, 0.18},
        {&ind.wealth, 0.16},
    };
}

}  // namespace

std::optional<double> compositeRating(const Indicators& indicators,
                                      const ClassifierConfig& cfg) {
    const auto metrics = positiveMetrics(indicators);

    double weightedSum = 0.0;
    double presentWeight = 0.0;
    int present = 0;
    for (const auto& wm : metrics) {
        if (wm.metric->hasValue()) {
            weightedSum += wm.metric->value() * wm.weight;
            presentWeight += wm.weight;
            ++present;
        }
    }

    if (present < cfg.minPositiveIndicators || presentWeight <= 0.0) {
        return std::nullopt;  // Not enough data — honour the "no fake data" rule.
    }
    return weightedSum / presentWeight;  // renormalise over present indicators
}

Classification classify(const District& district, const ClassifierConfig& cfg) {
    const Indicators& ind = district.indicators;
    Classification result;

    // 1) Hazard/risk zones take absolute priority (safety of the user first).
    //    They are only asserted when the corresponding risk metric is present,
    //    so a missing risk value never invents a hazard.
    if (ind.technogenicRisk.hasValue() &&
        ind.technogenicRisk.value() >= cfg.technogenicRiskThreshold) {
        result.zone = ZoneType::Red;
        result.sufficientData = true;
        result.reasonRu = "Высокий техногенный риск / вероятность ЧС.";
        result.rating = compositeRating(ind, cfg).value_or(0.0);
        return result;
    }
    if (ind.constructionRisk.hasValue() &&
        ind.constructionRisk.value() >= cfg.constructionRiskThreshold) {
        result.zone = ZoneType::Orange;
        result.sufficientData = true;
        result.reasonRu = "Сложные инженерно-геологические условия, риск просадки.";
        result.rating = compositeRating(ind, cfg).value_or(0.0);
        return result;
    }
    if (ind.healthHazard.hasValue() &&
        ind.healthHazard.value() >= cfg.healthHazardThreshold) {
        result.zone = ZoneType::Yellow;
        result.sufficientData = true;
        result.reasonRu = "Факторы опасности для здоровья (загрязнение, ЛЭП, шум).";
        result.rating = compositeRating(ind, cfg).value_or(0.0);
        return result;
    }
    if (ind.crime.hasValue() && ind.crime.value() >= cfg.crimeThreshold) {
        result.zone = ZoneType::Gray;
        result.sufficientData = true;
        result.reasonRu = "Высокий уровень преступности и неблагоприятная обстановка.";
        result.rating = compositeRating(ind, cfg).value_or(0.0);
        return result;
    }

    // 2) Otherwise fall back to the quality/business zones, which require a
    //    minimum amount of positive data to be present.
    const auto rating = compositeRating(ind, cfg);
    if (!rating) {
        result.zone = ZoneType::Unknown;
        result.sufficientData = false;
        result.reasonRu = "Недостаточно данных для классификации.";
        result.rating = 0.0;
        return result;
    }

    result.sufficientData = true;
    result.rating = *rating;

    // Business character: a strong business/service signal biases toward purple.
    const bool businessLeaning =
        ind.businessPotential.hasValue() && ind.service.hasValue() &&
        (ind.businessPotential.value() >= 65.0 && ind.service.value() >= 60.0);

    if (*rating >= cfg.greenScoreThreshold) {
        result.zone = ZoneType::Green;
        result.reasonRu = "Высокий уровень жизни, безопасность и инфраструктура.";
    } else if (businessLeaning || *rating >= cfg.purpleScoreThreshold) {
        result.zone = ZoneType::Purple;
        result.reasonRu = "Развитая коммерция и высокий уровень сервиса.";
    } else {
        result.zone = ZoneType::Blue;
        result.reasonRu = "Бюджетный сегмент, инфраструктура ниже среднего.";
    }
    return result;
}

std::vector<std::string> deriveRecommendations(const District& district) {
    std::vector<std::string> out;
    const Indicators& ind = district.indicators;

    auto present = [](const Metric& m) { return m.hasValue(); };

    if (present(ind.businessPotential) && ind.businessPotential.value() >= 65.0 &&
        present(ind.service) && ind.service.value() >= 60.0) {
        out.emplace_back(
            "Подходит для открытия кафе, ресторанов и сервисного бизнеса.");
    }
    if (present(ind.safety) && ind.safety.value() >= 70.0 &&
        present(ind.ecology) && ind.ecology.value() >= 65.0) {
        out.emplace_back(
            "Комфортная и безопасная территория для проживания с семьёй.");
    }
    if (present(ind.wealth) && ind.wealth.value() >= 70.0) {
        out.emplace_back(
            "Высокая покупательная способность — премиальный ритейл и услуги.");
    }
    if (present(ind.infrastructure) && ind.infrastructure.value() < 45.0) {
        out.emplace_back(
            "Ограниченная инфраструктура — учитывайте затраты на подключение.");
    }
    if (present(ind.crime) && ind.crime.value() >= 60.0) {
        out.emplace_back(
            "Повышенная преступность — рассмотрите инвестиции в безопасность.");
    }
    if (present(ind.healthHazard) && ind.healthHazard.value() >= 60.0) {
        out.emplace_back(
            "Присутствуют факторы риска для здоровья — не рекомендуется жильё.");
    }

    if (out.empty()) {
        out.emplace_back("Недостаточно данных для формирования рекомендаций.");
    }
    return out;
}

void applyClassification(District& district, const ClassifierConfig& cfg) {
    const auto c = classify(district, cfg);
    district.zone = c.zone;
    district.rating = c.rating;
    district.hasSufficientData = c.sufficientData;

    // Only (re)derive recommendations when we have not been given any.
    if (district.recommendations.empty()) {
        district.recommendations = deriveRecommendations(district);
    }
}

}  // namespace geobiz::core
