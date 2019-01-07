#pragma once

// system
#include <stdexcept>

namespace appimage {
    namespace desktop_integration {
        class DesktopIntegrationError : public std::runtime_error {
        public:
            explicit DesktopIntegrationError(const std::string& what) : runtime_error(what) {}
        };
    }
}
