#pragma once

// system
#include <stdexcept>

// local
#include <appimage/desktop_integration/Exceptions.h>

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            class DesktopEntryBuildError : public DesktopIntegrationError {
            public:
                explicit DesktopEntryBuildError(const std::string& what) : DesktopIntegrationError(what) {}
            };
        }
    }
}
