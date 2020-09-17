// STL includes
#include <algorithm>
#include <utility>
#include <vector>

// local includes
#include "StringSanitizer.h"

// initialize static const variables
const std::initializer_list<std::string::value_type> StringSanitizer::asciiLetters_ = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
};
const std::initializer_list<std::string::value_type> StringSanitizer::asciiDigits_ = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
};
const std::initializer_list<std::string::value_type> StringSanitizer::pathSafeChars_ = {
    '.', '-', '_'
};

StringSanitizer::StringSanitizer(std::string input) : input_(std::move(input)) {};

std::string StringSanitizer::sanitizeForPath() {
    // output buffer
    std::vector<std::string::value_type> buffer{};
    buffer.reserve(input_.size());

    // first of all, we compose an alphabet of safe characters
    // all characters not contained in this alphabet will be replaced by some safe character, e.g., an underscore (_)
    std::vector<std::string::value_type> safeAlphabet(asciiDigits_.size() + asciiLetters_.size() + pathSafeChars_.size());
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
