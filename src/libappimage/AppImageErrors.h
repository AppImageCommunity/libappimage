#pragma once

#include <stdexcept>

namespace appimage {
    class AppImageError : public std::runtime_error {
    public:
        AppImageError(const std::string& what) : runtime_error(what) {}
    };

    class WrongAppImageFormat : public AppImageError {
    public:
        WrongAppImageFormat(const std::string& what) : AppImageError(what) {}
    };

    class AppImageReadError : public AppImageError {
    public:
        AppImageReadError(const std::string& what) : AppImageError(what) {}
    };
};
