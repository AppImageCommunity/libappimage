// system
#include <map>

// local
#include <appimage/core/PayloadEntryType.h>
#include "ResourcesExtractor.h"

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            ResourcesExtractor::ResourcesExtractor(const core::AppImage& appImage) : appImage(appImage) {}

            DesktopIntegrationResources ResourcesExtractor::extract() {
                DesktopIntegrationResources resources = {};
                for (auto fileItr = appImage.files(); fileItr != fileItr.end(); ++fileItr) {
                    if (fileItr.type() != core::PayloadEntryType::REGULAR &&
                        fileItr.type() != core::PayloadEntryType::LINK)
                        continue;

                    const auto& filePath = *fileItr;

                    if (extractDesktopFile && isMainDesktopFile(filePath)) {
                        resources.desktopEntryPath = filePath;
                        resources.desktopEntryData = {std::istreambuf_iterator<char>(fileItr.read()),
                                                      std::istreambuf_iterator<char>()};

                        continue;
                    }

                    if ((extractIconFiles && isIconFile(filePath))) {
                        std::vector<char> data{std::istreambuf_iterator<char>(fileItr.read()),
                                               std::istreambuf_iterator<char>()};

                        if (!data.empty())
                            resources.icons[filePath] = data;

                        continue;
                    }

                    if ((extractMimeFiles && isMimeFile(filePath))) {
                        std::vector<char> data{std::istreambuf_iterator<char>(fileItr.read()),
                                               std::istreambuf_iterator<char>()};

                        if (!data.empty())
                            resources.mimeTypePackages[filePath] = data;

                        continue;
                    }

                    if ((extractAppDataFile && isAppDataFile(filePath))) {
                        resources.appStreamPath = filePath;
                        resources.appStreamData = {std::istreambuf_iterator<char>(fileItr.read()),
                                                   std::istreambuf_iterator<char>()};

                        continue;
                    }
                }


                return resources;
            }

            bool ResourcesExtractor::isAppDataFile(const std::string& filePath) const {
                return filePath.find("usr/share/metainfo/") != std::string::npos &&
                       filePath.find(".appdata.xml") != std::string::npos;
            }

            bool ResourcesExtractor::isIconFile(const std::string& fileName) const {
                return (fileName == ".DirIcon") ||
                       (fileName.find("usr/share/icons") != std::string::npos);
            }

            bool ResourcesExtractor::isMainDesktopFile(const std::string& fileName) const {
                return fileName.find(".desktop") != std::string::npos &&
                       fileName.find('/') == std::string::npos;
            }

            bool ResourcesExtractor::isMimeFile(const std::string& fileName) const {
                return fileName.find("usr/share/mime/packages") != std::string::npos &&
                       fileName.find(".xml") == std::string::npos;
            }

            void ResourcesExtractor::setExtractDesktopFile(bool extractDesktopFile) {
                ResourcesExtractor::extractDesktopFile = extractDesktopFile;
            }

            void ResourcesExtractor::setExtractIconFiles(bool extractIconFiles) {
                ResourcesExtractor::extractIconFiles = extractIconFiles;
            }

            void ResourcesExtractor::setExtractAppDataFile(bool extractAppDataFile) {
                ResourcesExtractor::extractAppDataFile = extractAppDataFile;
            }

            void ResourcesExtractor::setExtractMimeFiles(bool extractMimeFiles) {
                ResourcesExtractor::extractMimeFiles = extractMimeFiles;
            }
        }
    }
}
