#pragma once

// system
#include <stdexcept>

namespace appimage {
    namespace desktop_integration {
        class DesktopEntryBuildError : public std::runtime_error {
        public:
            explicit DesktopEntryBuildError(const std::string& what) : runtime_error(what) {}
        };
    }
}
