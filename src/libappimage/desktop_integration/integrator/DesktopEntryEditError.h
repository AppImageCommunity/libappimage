#pragma once

// system
#include <stdexcept>

// local
#include <appimage/desktop_integration/exceptions.h>

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            /**
             * Throw when something goes wrong with the desktop entry edition process.
             */
            class DesktopEntryEditError : public DesktopIntegrationError {
            public:
                explicit DesktopEntryEditError(const std::string& what) : DesktopIntegrationError(what) {}
            };
        }
    }
}
