#pragma once

#include <string>
#include <vector>
#include <map>

namespace appimage {
    namespace desktop_integration {
        /**
         * This entity is meant to hold the raw data of the resources required in the desktop integration process.
         * It should not be filled manually, instead use the ResourcesExtractor class.
         */
        struct DesktopIntegrationResources {
            std::string desktopEntryPath;
            std::vector<char> desktopEntryData;

            std::string appStreamPath;
            std::vector<char> appStreamData;

            std::map<std::string, std::vector<char>> icons;
            std::map<std::string, std::vector<char>> mimeTypePackages;
        };
    }
}
