#pragma once

// STL includes
#include <string>

/**
 * Sanitizes strings for different needs, such as e.g., for inclusion in filenames/paths.
 */
class StringSanitizer {
private:
    std::string input_;

    // these three lists can be used to compose alphabets for sanitization
    static constexpr std::initializer_list<std::string::value_type> asciiLetters_ = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    };
    static constexpr std::initializer_list<std::string::value_type> asciiDigits_ = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    };
    static constexpr std::initializer_list<std::string::value_type> pathSafeChars_ = {
        '.', '-', '_'
    };

public:
    /**
     * Default constructor.
     * @param input string to sanitize
     */
    explicit StringSanitizer(std::string  input);

    /**
     * Sanitizes given string so it is safe to embed it in a path.
     * Replaces all "unsafe" characters with a safe one.
     * The aim is to keep the string readable/understandable to the user.
     * @return sanitized filename
     */
    std::string sanitizeForPath();
};
