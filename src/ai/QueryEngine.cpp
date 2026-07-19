// SPDX-License-Identifier: MIT
#include "geobiz/ai/QueryEngine.hpp"

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#include "geobiz/core/Text.hpp"

namespace geobiz::ai {

namespace {

using core::District;
using core::Metric;

// UTF-8 (ASCII + Cyrillic) case folding so Russian queries match keyword tokens
// regardless of capitalisation ("Ташкента" → "ташкента").
std::string lower(const std::string& s) { return core::utf8Lower(s); }

bool has(const std::string& hay, const char* needle) {
    return hay.find(needle) != std::string::npos;
}

// A recognised geographic scope: the Cyrillic token the user is likely to type
// (robust to case endings via substring match) and the Latin form used in the
// dataset ids, so a Cyrillic query still filters the transliterated ids.
struct ScopeToken {
    const char* cyr;
    const char* lat;
};

// Extracts a geographic scope from a (lower-cased) query. Returns empty tokens
// when no known place is named ⇒ the whole country is in scope.
ScopeToken extractScope(const std::string& q) {
    // Common Uzbekistan regions/hubs mapped to their dataset id substrings.
    static const std::vector<ScopeToken> kKnown = {
        {"ташкент", "tashkent"},    {"самарканд", "samarkand"},
        {"бухар", "bukhara"},       {"андижан", "andijan"},
        {"ферган", "fergana"},      {"наманган", "namangan"},
        {"навои", "navoiy"},        {"джизак", "jizzakh"},
        {"термез", "surkhandarya"}, {"сурхандар", "surkhandarya"},
        {"карши", "kashkadarya"},   {"кашкадар", "kashkadarya"},
        {"гулистан", "syrdarya"},   {"сырдар", "syrdarya"},
        {"ургенч", "khorezm"},      {"хорезм", "khorezm"},
        {"нукус", "karakalpakstan"},{"каракалпак", "karakalpakstan"},
    };
    for (const auto& k : kKnown) {
        if (has(q, k.cyr)) return k;
    }
    return {nullptr, nullptr};
}

ParsedQuery parseImpl(const std::string& raw) {
    const std::string q = lower(raw);
    ParsedQuery pq;
    if (const ScopeToken s = extractScope(q); s.cyr != nullptr) {
        pq.scopeCyrillic = s.cyr;
        pq.scopeLatin = s.lat;
    }

    // Order matters: more specific intents first.
    if (has(q, "кофейн") || has(q, "кофе") || has(q, "coffee")) {
        pq.intent = Intent::OpenCoffeeShop;
    } else if (has(q, "ресторан") || has(q, "кафе") || has(q, "restaurant")) {
        pq.intent = Intent::OpenRestaurant;
    } else if (has(q, "конкуренц")) {
        pq.intent = Intent::LowCompetition;
    } else if (has(q, "богат") || has(q, "достат") || has(q, "премиум")) {
        pq.intent = Intent::Wealthiest;
    } else if (has(q, "безопасн")) {
        pq.intent = Intent::Safest;
    } else if (has(q, "эколог") || has(q, "чист") || has(q, "воздух")) {
        pq.intent = Intent::BestEcology;
    } else if (has(q, "инфраструктур")) {
        pq.intent = Intent::BestInfrastructure;
    } else if (has(q, "бизнес") || has(q, "открыть") || has(q, "магазин") ||
               has(q, "торгов")) {
        pq.intent = Intent::OpenBusinessGeneric;
    }
    return pq;
}

// Returns true when `d` (by name) falls within the requested scope. A district
// matches when its own or its parent chain's name contains the scope token. To
// keep the engine self-contained we approximate the chain check by matching the
// district's own localized names (data files place the region token in child
// names or provide flat regional data), plus id prefix.
bool inScope(const District& d, const ParsedQuery& pq) {
    if (!pq.hasScope()) return true;

    // Match localized names against the Cyrillic token…
    auto nameHit = [&](const std::string& n) {
        return !n.empty() && lower(n).find(pq.scopeCyrillic) != std::string::npos;
    };
    if (nameHit(d.nameRu) || nameHit(d.nameEn) || nameHit(d.nameUz)) return true;

    // …and the transliterated id / parent chain against the Latin token, since
    // ids look like "uz.tashkent-city.yunusabad".
    if (!pq.scopeLatin.empty()) {
        if (lower(d.id).find(pq.scopeLatin) != std::string::npos) return true;
        if (lower(d.parentId).find(pq.scopeLatin) != std::string::npos) return true;
    }
    return false;
}

// A scoring objective: given a district produce an optional score. Missing when
// the required indicator is absent so the district drops out of the ranking.
using Objective = std::function<std::optional<double>(const District&)>;

std::optional<double> avgPresent(std::initializer_list<const Metric*> metrics) {
    double sum = 0.0;
    int n = 0;
    for (const Metric* m : metrics) {
        if (m->hasValue()) {
            sum += m->value();
            ++n;
        }
    }
    if (n == 0) return std::nullopt;
    return sum / n;
}

Objective objectiveFor(Intent intent) {
    switch (intent) {
        case Intent::OpenCoffeeShop:
        case Intent::OpenRestaurant:
            return [](const District& d) {
                return avgPresent({&d.indicators.businessPotential,
                                   &d.indicators.service, &d.indicators.wealth,
                                   &d.indicators.safety});
            };
        case Intent::OpenBusinessGeneric:
            return [](const District& d) {
                return avgPresent({&d.indicators.businessPotential,
                                   &d.indicators.infrastructure,
                                   &d.indicators.wealth});
            };
        case Intent::LowCompetition:
            // "Меньше конкуренция" ≈ solid demand (wealth/population-driven
            // business potential) but modest existing service saturation. We
            // reward high businessPotential and *low* service, both required.
            return [](const District& d) -> std::optional<double> {
                if (!d.indicators.businessPotential.hasValue() ||
                    !d.indicators.service.hasValue()) {
                    return std::nullopt;
                }
                const double demand = d.indicators.businessPotential.value();
                const double saturation = d.indicators.service.value();
                return std::clamp(demand - 0.5 * saturation, 0.0, 100.0);
            };
        case Intent::Wealthiest:
            return [](const District& d) -> std::optional<double> {
                return d.indicators.wealth.hasValue()
                           ? std::optional<double>(d.indicators.wealth.value())
                           : std::nullopt;
            };
        case Intent::Safest:
            return [](const District& d) -> std::optional<double> {
                return d.indicators.safety.hasValue()
                           ? std::optional<double>(d.indicators.safety.value())
                           : std::nullopt;
            };
        case Intent::BestEcology:
            return [](const District& d) -> std::optional<double> {
                return d.indicators.ecology.hasValue()
                           ? std::optional<double>(d.indicators.ecology.value())
                           : std::nullopt;
            };
        case Intent::BestInfrastructure:
            return [](const District& d) -> std::optional<double> {
                return d.indicators.infrastructure.hasValue()
                           ? std::optional<double>(
                                 d.indicators.infrastructure.value())
                           : std::nullopt;
            };
        case Intent::Unknown:
        default:
            return {};
    }
}

const char* intentHeadline(Intent intent) {
    switch (intent) {
        case Intent::OpenCoffeeShop:
            return "Районы, наиболее подходящие для открытия кофейни";
        case Intent::OpenRestaurant:
            return "Лучшие районы для ресторана или кафе";
        case Intent::OpenBusinessGeneric:
            return "Районы с наибольшим бизнес-потенциалом";
        case Intent::LowCompetition:
            return "Районы со спросом и невысокой насыщенностью сервисом";
        case Intent::Wealthiest:
            return "Самые обеспеченные районы";
        case Intent::Safest:
            return "Самые безопасные районы";
        case Intent::BestEcology:
            return "Районы с лучшей экологией";
        case Intent::BestInfrastructure:
            return "Районы с самой развитой инфраструктурой";
        default:
            return "";
    }
}

std::string rationaleFor(Intent intent, const District& d) {
    switch (intent) {
        case Intent::OpenCoffeeShop:
        case Intent::OpenRestaurant:
        case Intent::OpenBusinessGeneric:
            return "Высокий бизнес-потенциал и уровень сервиса.";
        case Intent::LowCompetition:
            return "Хороший спрос при умеренной конкуренции по сервису.";
        case Intent::Wealthiest:
            return "Высокий уровень достатка населения.";
        case Intent::Safest:
            return "Высокий показатель безопасности.";
        case Intent::BestEcology:
            return "Благоприятная экологическая обстановка.";
        case Intent::BestInfrastructure:
            return "Развитая инженерная и социальная инфраструктура.";
        default:
            return {};
    }
    (void)d;
}

}  // namespace

ParsedQuery QueryEngine::parse(const std::string& rawQuery) {
    return parseImpl(rawQuery);
}

Answer QueryEngine::ask(const std::string& rawQuery, std::size_t topN) const {
    const ParsedQuery pq = parseImpl(rawQuery);
    Answer answer;

    if (pq.intent == Intent::Unknown) {
        answer.sufficientData = false;
        answer.summaryRu =
            "Не удалось понять вопрос. Попробуйте, например: «Где лучше открыть "
            "кофейню?», «Самые безопасные районы», «Самые богатые районы "
            "Ташкента».";
        return answer;
    }

    const Objective objective = objectiveFor(pq.intent);
    if (!objective) {
        answer.sufficientData = false;
        answer.summaryRu = "Недостаточно данных для ответа на этот вопрос.";
        return answer;
    }

    std::vector<Suggestion> scored;
    for (const District& d : districts_) {
        // We rank leaf-ish territories; skip the country root.
        if (d.level == core::AdminLevel::Country) continue;
        if (!inScope(d, pq)) continue;

        const auto score = objective(d);
        if (!score) continue;  // indicator missing ⇒ do not fabricate a ranking.

        scored.push_back({d.id, d.nameRu, *score, rationaleFor(pq.intent, d)});
    }

    if (scored.empty()) {
        answer.sufficientData = false;
        answer.summaryRu =
            pq.hasScope() ? "Недостаточно данных по выбранной территории."
                          : "Недостаточно данных для ответа на этот вопрос.";
        return answer;
    }

    std::stable_sort(scored.begin(), scored.end(),
                     [](const Suggestion& a, const Suggestion& b) {
                         return a.score > b.score;
                     });
    if (scored.size() > topN) scored.resize(topN);

    answer.summaryRu = intentHeadline(pq.intent);
    answer.suggestions = std::move(scored);
    return answer;
}

}  // namespace geobiz::ai
