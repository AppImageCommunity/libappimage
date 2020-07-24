// STL includes
#include <algorithm>
#include <utility>
#include <vector>

// local includes
#include "StringSanitizer.h"

StringSanitizer::StringSanitizer(std::string  input) : input_(std::move(input)) {};

std::string StringSanitizer::sanitizeForPath() {
    // output buffer
    std::vector<std::string::value_type> buffer{};
    buffer.reserve(input_.size());

    // first of all, we compose an alphabet of safe characters
    // all characters not contained in this alphabet will be replaced by some safe character, e.g., an underscore (_)
    std::vector<std::string::value_type> safeAlphabet{asciiDigits_.size() + asciiLetters_.size() + pathSafeChars_.size()};
    for (const auto& partialAlphabet : {asciiDigits_, asciiLetters_, pathSafeChars_}) {
        std::copy(partialAlphabet.begin(), partialAlphabet.end(), std::back_inserter(safeAlphabet));
    }

    for (auto c : input_) {
        // replace if c is not an element of the safe alphabet
        if (std::find(safeAlphabet.begin(), safeAlphabet.end(), c) == safeAlphabet.end()) {
            c = '_';
        }

        buffer.emplace_back(c);
    }

    // C strings, anyone?
    buffer.emplace_back('\0');

    return std::string{buffer.data()};
}
