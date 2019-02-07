#pragma once

#include <string>
#include <vector>
#include <map>

namespace appimage {
    namespace desktop_integration {
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
