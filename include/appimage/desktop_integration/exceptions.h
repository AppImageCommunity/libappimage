#pragma once

// local
#include <appimage/core/exceptions.h>

namespace appimage {
    namespace desktop_integration {

        /**
         * Throw in case of failure while performing a given desktop integration operation.
         * For example when a DesktopEntry is missing or malformed.
         */
        class DesktopIntegrationError : public core::AppImageError {
        public:
            explicit DesktopIntegrationError(const std::string& what) : core::AppImageError(what) {}
        };
    }
}
