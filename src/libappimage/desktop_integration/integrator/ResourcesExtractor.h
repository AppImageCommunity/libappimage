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
                explicit ResourcesExtractor(core::AppImage appImage);

                DesktopIntegrationResources extract();

                void setExtractDesktopFile(bool extractDesktopFile);

                void setExtractIconFiles(bool extractIconFiles);

                void setExtractAppDataFile(bool extractAppDataFile);

                void setExtractMimeFiles(bool extractMimeFiles);

            private:
                bool extractDesktopFile = true;
                bool extractIconFiles = true;
                bool extractAppDataFile = true;
                bool extractMimeFiles = true;

                core::AppImage appImage;

                bool isIconFile(const std::string& fileName) const;

                bool isMainDesktopFile(const std::string& fileName) const;

                bool isAppDataFile(const std::string& filePath) const;

                bool isMimeFile(const std::string& filePath) const;
            };
        }
    }
}
