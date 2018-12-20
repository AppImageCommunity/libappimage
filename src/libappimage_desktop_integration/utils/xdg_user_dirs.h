#pragma once

// system
#include <string>

namespace appimage {
    namespace utils {
        /**
         * @brief Provide access to the user directories listed at
         * https://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
         *
         * This is not meant to be a feature complete implementation of the XDG BASE DIR specification.
         */
        class xdg_user_dirs {
        public:
            static std::string data();
        };
    }
}


