// system
#include <map>
#include <set>

// libraries
#include <boost/filesystem.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include <appimage/core/PayloadEntryType.h>
#include <appimage/desktop_integration/exceptions.h>
#include "ResourcesExtractor.h"

using namespace XdgUtils::DesktopEntry;
using namespace appimage::core;
namespace bf = boost::filesystem;

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            ResourcesExtractor::ResourcesExtractor(const core::AppImage& appImage) : appImage(appImage),
                                                                                     entriesCache(appImage) {}

            DesktopIntegrationResources ResourcesExtractor::extract() {

                PayloadEntriesCache entriesCache(appImage);

                /*
                 * Use the cache to identify the resources entries that must be extracted. In case of LINK entries
                 * store the final link target which must point to a REGULAR entry to ensure a proper extraction
                 * by means of `PayloadIterator::read`.
                 */
                DesktopIntegrationResources resources = {};
                resolveResourcesFinalPaths(resources, entriesCache);

                /*
                 * Call `PayloadIterator::read` on each entry included in resources and store the value in the
                 * `data` members or in the right side of the maps.
                 */
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

            DesktopEntry ResourcesExtractor::extractDesktopEntry() {
                for (auto fileItr = appImage.files(); fileItr != fileItr.end(); ++fileItr)
                    if (isMainDesktopFile(fileItr.path()))
                        return DesktopEntry(fileItr.read());

                throw DesktopIntegrationError("Missing Desktop Entry");
            }

            std::vector<std::string> ResourcesExtractor::getIconFilePaths(const std::string& iconName) {
                std::vector<std::string> filePaths;

                for (const auto& filePath: entriesCache.getEntriesPaths()) {
                    if (filePath.find("usr/share/icons") != std::string::npos &&
                        filePath.find(iconName) != std::string::npos) {
                        filePaths.emplace_back(filePath);
                    }
                }

                return filePaths;
            }

            std::vector<std::string> ResourcesExtractor::getMimeTypePackagesPaths() {
                std::vector<std::string> filePaths;

                for (const auto& filePath: entriesCache.getEntriesPaths()) {
                    if (isMimeFile(filePath))
                        filePaths.emplace_back(filePath);
                }

                return filePaths;
            }

            void ResourcesExtractor::extractEntriesTo(const std::map<std::string, std::string>& targetsMap) {
                // Resolve links to ensure proper extraction
                std::map<std::string, std::string> realTargetsMap;
                for (const auto& target: targetsMap) {
                    if (entriesCache.getEntryType(target.first) == PayloadEntryType::LINK) {
                        const std::string& realTarget = entriesCache.getEntryLinkTarget(target.first);
                        realTargetsMap[realTarget] = target.second;
                    } else {
                        realTargetsMap.insert(target);
                    }
                }


                for (auto fileItr = appImage.files(); fileItr != fileItr.end(); ++fileItr) {
                    const auto deployPathMapping = realTargetsMap.find(fileItr.path());
                    if (deployPathMapping != realTargetsMap.end()) {
                        bf::path deployPath(deployPathMapping->second);

                        // create parent dirs
                        const auto parentDirPath = deployPath.parent_path();
                        bf::create_directories(parentDirPath);

                        // write file contents
                        bf::ofstream file(deployPath);
                        file << fileItr.read().rdbuf();

                        file.close();
                    }
                }

            }

            std::vector<char> ResourcesExtractor::extractFile(const std::string& path) {
                // Resolve any link before extracting the file
                auto regularEntryPath = path;
                if (entriesCache.getEntryType(path) == PayloadEntryType::LINK)
                    regularEntryPath = entriesCache.getEntryLinkTarget(path);

                for (auto fileItr = appImage.files(); fileItr != fileItr.end(); ++fileItr) {
                    if (fileItr.path() == regularEntryPath)
                        return readWholeFile(fileItr.read());
                }

                throw core::PayloadIteratorError("Entry doesn't exists: " + path);
            }

            std::map<std::string, std::vector<char>>
            ResourcesExtractor::extractFiles(const std::vector<std::string>& paths) {
                // Resolve any link before extracting the files and keep a reference to the original path
                std::map<std::string, std::string> reverseLinks;
                for (const auto& path: paths)
                    if (entriesCache.getEntryType(path) == PayloadEntryType::LINK)
                        reverseLinks[entriesCache.getEntryLinkTarget(path)] = path;
                    else
                        reverseLinks[path] = path;

                std::map<std::string, std::vector<char>> result;
                for (auto fileItr = appImage.files(); fileItr != fileItr.end(); ++fileItr) {
                    auto itr = reverseLinks.find(fileItr.path());

                    // extract the file data and store it using the original path
                    if (itr != reverseLinks.end())
                        result[itr->second] = readWholeFile(fileItr.read());
                }

                return result;

            }
        }
    }
}
