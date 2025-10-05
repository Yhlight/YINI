#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace YINI::StringUtils {
/**
 * @brief Calculates the Levenshtein distance between two strings.
 *
 * The Levenshtein distance is the minimum number of single-character edits
 * (insertions, deletions, or substitutions) required to change one word into the other.
 *
 * @param s1 The first string.
 * @param s2 The second string.
 * @return The Levenshtein distance between s1 and s2.
 */
inline int levenshtein_distance(const std::string& s1, const std::string& s2) {
    const size_t len1 = s1.size(), len2 = s2.size();
    std::vector<unsigned int> col(len2 + 1), prev_col(len2 + 1);

    for (unsigned int i = 0; i < prev_col.size(); i++) {
        prev_col[i] = i;
    }

    for (unsigned int i = 0; i < len1; i++) {
        col[0] = i + 1;
        for (unsigned int j = 0; j < len2; j++) {
            col[j + 1] = std::min({prev_col[j + 1] + 1, col[j] + 1, prev_col[j] + (s1[i] == s2[j] ? 0 : 1)});
        }
        col.swap(prev_col);
    }
    return prev_col[len2];
}

/**
 * @brief Finds the most similar string to a given target from a list of candidates.
 *
 * This function iterates through a vector of candidate strings and returns the one
 * with the smallest Levenshtein distance to the target string.
 *
 * @param target The string to find a match for.
 * @param candidates A vector of strings to search through.
 * @return The most similar string from the candidates. Returns an empty string if candidates is empty.
 */
inline std::string find_most_similar(const std::string& target, const std::vector<std::string>& candidates) {
    if (candidates.empty()) {
        return "";
    }

    std::string best_match = "";
    int min_distance = -1;

    for (const auto& candidate : candidates) {
        int distance = levenshtein_distance(target, candidate);
        if (min_distance == -1 || distance < min_distance) {
            min_distance = distance;
            best_match = candidate;
        }
    }

    return best_match;
}
}  // namespace YINI::StringUtils