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
    static const std::initializer_list<std::string::value_type> asciiLetters_;
    static const std::initializer_list<std::string::value_type> asciiDigits_;
    static const std::initializer_list<std::string::value_type> pathSafeChars_;

public:
    /**
     * Default constructor.
     * @param input string to sanitize
     */
    explicit StringSanitizer(std::string input);

    /**
     * Sanitizes given string so it is safe to embed it in a path.
     * Replaces all "unsafe" characters with a safe one.
     * The aim is to keep the string readable/understandable to the user.
     * @return sanitized filename
     */
    std::string sanitizeForPath();
};
