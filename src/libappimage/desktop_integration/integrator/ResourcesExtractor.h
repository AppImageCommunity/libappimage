#pragma once

// system
#include <string>

// libraries
#include <appimage/core/AppImage.h>

// local
#include "DesktopIntegrationResources.h"

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            /**
             * Extract the resources (files) required to integrate an AppImage into the desktop environment
             * in an efficient way.
             */
            class ResourcesExtractor {
            public:
                explicit ResourcesExtractor(const core::AppImage& appImage);

                DesktopIntegrationResources extract();

                void setExtractDesktopFile(bool extractDesktopFile);

                void setExtractIconFiles(bool extractIconFiles);

                void setExtractAppDataFile(bool extractAppDataFile);

                void setExtractMimeFiles(bool extractMimeFiles);

            private:
                bool extractDesktopFile = false;
                bool extractIconFiles = false;
                bool extractAppDataFile = false;
                bool extractMimeFiles = false;

                core::AppImage appImage;

                bool isIconFile(const std::string& fileName) const;

                bool isMainDesktopFile(const std::string& fileName) const;

                bool isAppDataFile(const std::string& filePath) const;

                bool isMimeFile(const std::string& filePath) const;
            };
        }
    }
}
