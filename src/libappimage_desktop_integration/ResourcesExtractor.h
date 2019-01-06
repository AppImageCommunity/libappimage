#pragma once

// system
#include <string>

// libraries
#include <appimage/core/AppImage.h>

namespace appimage {
    namespace desktop_integration {

        typedef std::map<std::string, std::vector<char>> DesktopIntegrationResources;

        class ResourcesExtractor {
            bool extractDesktopFile = true;
            bool extractIconFiles = true;
            bool extractAppDataFile = true;
            bool extractMimeFiles = true;

            std::string appImagePath;
            std::unique_ptr<core::AppImage> appImage;

        public:
            explicit ResourcesExtractor(const std::string& appImagePath);

            DesktopIntegrationResources extract();

            void setExtractDesktopFile(bool extractDesktopFile);

            void setExtractIconFiles(bool extractIconFiles);

            void setExtractAppDataFile(bool extractAppDataFile);

            void setExtractMimeFiles(bool extractMimeFiles);

        private:
            bool isIconFile(const std::string& fileName) const;

            bool isMainDesktopFile(const std::string& fileName) const;

            bool isAppDataFile(const std::string& filePath) const;

            bool isMimeFile(const std::string& filePath) const;
        };
    }
}
