// system
#include <map>

// local
#include "ResourcesExtractor.h"

namespace appimage {
    namespace desktop_integration {
        ResourcesExtractor::ResourcesExtractor(const std::string& appImagePath) : appImagePath(appImagePath) {}

        std::map<std::string, std::vector<char>> ResourcesExtractor::extract() {
            appImage.reset(new core::AppImage(appImagePath));
            std::map<std::string, std::vector<char>> resources;
            for (auto fileItr = appImage->files(); fileItr != fileItr.end(); ++fileItr) {
                const auto& filePath = *fileItr;

                if ((extractDesktopFile && isMainDesktopFile(filePath)) ||
                    (extractIconFiles && isIconFile(filePath)) ||
                    (extractAppDataFile && isAppDataFile(filePath)) ||
                    (extractMimeFiles && isMimeFile(filePath))) {
                    std::vector<char> data{std::istreambuf_iterator<char>(fileItr.read()),
                                           std::istreambuf_iterator<char>()};

                    resources[filePath] = data;
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
                   fileName.find("usr/share/icons") != std::string::npos;
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
