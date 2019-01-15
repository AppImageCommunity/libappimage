#pragma once

// system
#include <string>
#include <stdexcept>

namespace appimage {
    namespace desktop_integration {

        namespace thumbnailer {
            class ThumbnailerError : public std::runtime_error {
            public:
                ThumbnailerError(const std::string& what) : runtime_error(what) {}
            };

            class CImgError : public ThumbnailerError {
            public:
                CImgError(const std::string& what) : ThumbnailerError(what) {}
            };

            class RSvgError : public ThumbnailerError {
            public:
                RSvgError(const std::string& what) : ThumbnailerError(what) {}
            };
        }
    }
}
