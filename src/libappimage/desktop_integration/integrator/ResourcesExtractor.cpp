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
                PayloadEntriesCache entriesCache(appImage);

                DesktopIntegrationResources resources = {};
                resolveResourcesFinalPaths(resources, entriesCache);
                readResourceFiles(resources);

                if (entriesCache.getEntryType(".DirIcon") == core::PayloadEntryType::LINK) {
                    /**
                    * .DirIcon is expected to exists in every AppImage but if it's a symlink its path in the
                    * path in resource.icons will be the one corresponding to the target.
                    *
                    * Ensure that there is a reference to ".DirIcon"
                    */

                    auto dirIconFinalPath = entriesCache.getEntryLinkTarget(".DirIcon");
                    resources.icons[".DirIcon"] = resources.icons[dirIconFinalPath];
                }
                return resources;
            }

            void ResourcesExtractor::readResourceFiles(DesktopIntegrationResources& resources) {
                for (auto fileItr = appImage.files(); fileItr != fileItr.end(); ++fileItr) {
                    const auto& filePath = *fileItr;

                    // Read desktop entry
                    if (resources.desktopEntryPath == filePath) {
                        resources.desktopEntryData = readWholeFile(fileItr.read());
                        continue;
                    }

                    // Read AppStream file
                    if (resources.appStreamPath == filePath) {
                        resources.appStreamData = readWholeFile(fileItr.read());
                        continue;
                    }

                    // Read Icons
                    auto iconsItr = resources.icons.find(filePath);
                    if (iconsItr != resources.icons.end()) {
                        std::vector<char> data = readWholeFile(fileItr.read());

                        // Don't store emtpy files, they are useless
                        if (data.empty())
                            resources.icons.erase(iconsItr);
                        else
                            iconsItr->second = data;

                        continue;
                    }

                    // Read Mime Type Packages
                    auto mimeTypePackagesItr = resources.mimeTypePackages.find(filePath);
                    if (mimeTypePackagesItr != resources.mimeTypePackages.end()) {
                        std::vector<char> data = readWholeFile(fileItr.read());

                        // Don't store emtpy files, they are useless
                        if (data.empty())
                            resources.icons.erase(iconsItr);
                        else
                            mimeTypePackagesItr->second = data;

                        continue;
                    }
                }
            }

            void ResourcesExtractor::resolveResourcesFinalPaths(DesktopIntegrationResources& resources,
                                                                PayloadEntriesCache entriesCache) {
                auto entriesPaths = entriesCache.getEntriesPaths();
                for (const auto& path: entriesPaths) {
                    auto entryType = entriesCache.getEntryType(path);
                    if (entryType != core::PayloadEntryType::REGULAR &&
                        entryType != core::PayloadEntryType::LINK) {
                        continue;
                    }

                    // Use the final path in case of links
                    auto finalPath = path;
                    if (entryType == core::PayloadEntryType::LINK)
                        finalPath = entriesCache.getEntryLinkTarget(path);

                    if (extractDesktopFile && isMainDesktopFile(path))
                        resources.desktopEntryPath = finalPath;

                    if ((extractIconFiles && isIconFile(path)))
                        resources.icons[finalPath] = {};

                    if ((extractMimeFiles && isMimeFile(path)))
                        resources.mimeTypePackages[finalPath] = {};

                    if ((extractAppDataFile && isAppDataFile(path)))
                        resources.appStreamPath = finalPath;
                }
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

            std::vector<char> ResourcesExtractor::readWholeFile(std::istream& istream) {
                return {std::istreambuf_iterator<char>(istream), std::istreambuf_iterator<char>()};
            }
        }
    }
}
