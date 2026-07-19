// SPDX-License-Identifier: MIT
//
// DistrictRepository.hpp — in-memory store + query surface over all districts.
//
// The repository aggregates districts from one or more providers, indexes them
// for fast lookup, runs classification, and exposes the read queries the UI and
// the AI engine need. It owns no Qt types so it stays unit-testable.

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "geobiz/core/Classifier.hpp"
#include "geobiz/core/District.hpp"
#include "geobiz/data/IDataProvider.hpp"

namespace geobiz::data {

/// Thread-compatible (not thread-safe) repository. Loading is expected to happen
/// on a worker thread; once populated the read methods are safe for concurrent
/// readers provided no concurrent load() is in flight.
class DistrictRepository {
public:
    explicit DistrictRepository(core::ClassifierConfig cfg = {});

    /// Registers a provider. Providers are consulted in registration order; the
    /// first is treated as the source of truth, the rest as enrichment.
    void addProvider(DataProviderPtr provider);

    /// (Re)loads every district from the registered providers, runs enrichment
    /// and classification. Returns the number of districts loaded. Attribution
    /// and any per-provider messages are collected in `lastMessages()`.
    std::size_t reload();

    [[nodiscard]] const std::vector<core::District>& districts() const noexcept {
        return districts_;
    }

    /// O(1) lookup by id; nullptr when unknown.
    [[nodiscard]] const core::District* find(const std::string& id) const;

    /// Children of a territory (one level down).
    [[nodiscard]] std::vector<const core::District*> childrenOf(
        const std::string& parentId) const;

    /// Case-insensitive, accent-naive substring search over the localized names.
    /// Results are ordered by relevance (prefix matches first).
    [[nodiscard]] std::vector<const core::District*> search(
        const std::string& query, std::size_t limit = 25) const;

    [[nodiscard]] const std::vector<std::string>& lastMessages() const noexcept {
        return messages_;
    }

private:
    void reindex();

    core::ClassifierConfig                                  cfg_;
    std::vector<DataProviderPtr>                            providers_;
    std::vector<core::District>                             districts_;
    std::unordered_map<std::string, std::size_t>           indexById_;
    std::vector<std::string>                                messages_;
};

}  // namespace geobiz::data
