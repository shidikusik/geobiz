// SPDX-License-Identifier: MIT
#include "geobiz/data/DistrictRepository.hpp"

#include <algorithm>
#include <utility>

#include "geobiz/core/Text.hpp"

namespace geobiz::data {

namespace {

// UTF-8 (ASCII + Cyrillic) case folding, shared with the AI engine, so search
// works for Russian names regardless of capitalisation.
std::string toLower(const std::string& s) { return core::utf8Lower(s); }

bool containsCi(const std::string& haystack, const std::string& needleLower) {
    if (needleLower.empty()) return true;
    return toLower(haystack).find(needleLower) != std::string::npos;
}

bool startsWithCi(const std::string& haystack, const std::string& needleLower) {
    return toLower(haystack).rfind(needleLower, 0) == 0;
}

}  // namespace

DistrictRepository::DistrictRepository(core::ClassifierConfig cfg)
    : cfg_(std::move(cfg)) {}

void DistrictRepository::addProvider(DataProviderPtr provider) {
    if (provider) providers_.push_back(std::move(provider));
}

std::size_t DistrictRepository::reload() {
    districts_.clear();
    messages_.clear();

    bool first = true;
    for (const auto& provider : providers_) {
        if (first) {
            // Source of truth — provides the district set.
            DatasetResult r = provider->load();
            messages_.push_back(provider->name() + ": " + r.message);
            if (r.ok) {
                districts_ = std::move(r.districts);
            }
            first = false;
        } else {
            // Enrichment provider — augments existing districts in place.
            provider->enrich(districts_);
            messages_.push_back(provider->name() + ": enrichment applied");
        }
    }

    // Classify every district after all enrichment has been applied.
    for (auto& d : districts_) {
        core::applyClassification(d, cfg_);
    }

    reindex();
    return districts_.size();
}

void DistrictRepository::reindex() {
    indexById_.clear();
    indexById_.reserve(districts_.size());
    for (std::size_t i = 0; i < districts_.size(); ++i) {
        indexById_.emplace(districts_[i].id, i);
    }
}

const core::District* DistrictRepository::find(const std::string& id) const {
    const auto it = indexById_.find(id);
    if (it == indexById_.end()) return nullptr;
    return &districts_[it->second];
}

std::vector<const core::District*> DistrictRepository::childrenOf(
    const std::string& parentId) const {
    std::vector<const core::District*> out;
    for (const auto& d : districts_) {
        if (d.parentId == parentId) out.push_back(&d);
    }
    return out;
}

std::vector<const core::District*> DistrictRepository::search(
    const std::string& query, std::size_t limit) const {
    const std::string q = toLower(query);

    struct Scored {
        const core::District* d;
        int                   score;
    };
    std::vector<Scored> hits;

    for (const auto& d : districts_) {
        int best = -1;
        for (const std::string* name : {&d.nameRu, &d.nameUz, &d.nameEn}) {
            if (name->empty()) continue;
            if (startsWithCi(*name, q)) {
                best = std::max(best, 2);
            } else if (containsCi(*name, q)) {
                best = std::max(best, 1);
            }
        }
        if (best >= 0) hits.push_back({&d, best});
    }

    std::stable_sort(hits.begin(), hits.end(),
                     [](const Scored& a, const Scored& b) {
                         return a.score > b.score;
                     });

    std::vector<const core::District*> out;
    out.reserve(std::min(limit, hits.size()));
    for (const auto& h : hits) {
        if (out.size() >= limit) break;
        out.push_back(h.d);
    }
    return out;
}

}  // namespace geobiz::data
