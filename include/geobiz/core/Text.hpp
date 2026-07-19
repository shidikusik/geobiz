// SPDX-License-Identifier: MIT
//
// Text.hpp — minimal, dependency-free text helpers for case-insensitive search.
//
// The UI language is Russian, so naive ASCII lower-casing is not enough: a query
// like "Ташкент" must match the token "ташкент". `utf8Lower` folds ASCII plus
// the common Cyrillic ranges (У+0410..У+044F and Ё/ё) without pulling in ICU,
// keeping the core library free of heavy dependencies.

#pragma once

#include <string>

namespace geobiz::core {

/// Lower-cases an UTF-8 string across ASCII and the common Cyrillic letters.
/// Bytes outside those ranges are passed through unchanged.
[[nodiscard]] inline std::string utf8Lower(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size();) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c < 0x80) {
            // ASCII
            out.push_back(static_cast<char>(
                (c >= 'A' && c <= 'Z') ? c + 0x20 : c));
            ++i;
        } else if (c == 0xD0 && i + 1 < s.size()) {
            const unsigned char c2 = static_cast<unsigned char>(s[i + 1]);
            if (c2 >= 0x90 && c2 <= 0x9F) {
                // А..П -> а..п
                out.push_back(static_cast<char>(0xD0));
                out.push_back(static_cast<char>(c2 + 0x20));
            } else if (c2 >= 0xA0 && c2 <= 0xAF) {
                // Р..Я -> р..я (lead byte flips to 0xD1)
                out.push_back(static_cast<char>(0xD1));
                out.push_back(static_cast<char>(c2 - 0x20));
            } else if (c2 == 0x81) {
                // Ё -> ё
                out.push_back(static_cast<char>(0xD1));
                out.push_back(static_cast<char>(0x91));
            } else {
                out.push_back(static_cast<char>(c));
                out.push_back(static_cast<char>(c2));
            }
            i += 2;
        } else if (c >= 0xC0 && i + 1 < s.size()) {
            // Other 2-byte UTF-8 sequence — copy verbatim.
            out.push_back(static_cast<char>(c));
            out.push_back(s[i + 1]);
            i += 2;
        } else {
            out.push_back(static_cast<char>(c));
            ++i;
        }
    }
    return out;
}

}  // namespace geobiz::core
