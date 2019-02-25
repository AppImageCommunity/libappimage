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

            bool ResourcesExtractor::isIconFile(const std::string& fileName) const {
                return (fileName.find("usr/share/icons") != std::string::npos);
            }

            bool ResourcesExtractor::isMainDesktopFile(const std::string& fileName) const {
                return fileName.find(".desktop") != std::string::npos &&
                       fileName.find('/') == std::string::npos;
            }

            bool ResourcesExtractor::isMimeFile(const std::string& fileName) const {
                return fileName.find("usr/share/mime/packages") != std::string::npos &&
                       fileName.find(".xml") == std::string::npos;
            }

            std::vector<char> ResourcesExtractor::readWholeFile(std::istream& istream) const {
                return {std::istreambuf_iterator<char>(istream), std::istreambuf_iterator<char>()};
            }

            DesktopEntry ResourcesExtractor::extractDesktopEntry() const {
                for (auto fileItr = appImage.files(); fileItr != fileItr.end(); ++fileItr)
                    if (isMainDesktopFile(fileItr.path()))
                        return DesktopEntry(fileItr.read());

                throw DesktopIntegrationError("Missing Desktop Entry");
            }

            std::vector<std::string> ResourcesExtractor::getIconFilePaths(const std::string& iconName) const {
                std::vector<std::string> filePaths;

                for (const auto& filePath: entriesCache.getEntriesPaths()) {
                    if (isIconFile(filePath) &&
                        filePath.find(iconName) != std::string::npos) {
                        filePaths.emplace_back(filePath);
                    }
                }

                return filePaths;
            }

            std::vector<std::string> ResourcesExtractor::getMimeTypePackagesPaths() const {
                std::vector<std::string> filePaths;

                for (const auto& filePath: entriesCache.getEntriesPaths()) {
                    if (isMimeFile(filePath))
                        filePaths.emplace_back(filePath);
                }

                return filePaths;
            }

            void ResourcesExtractor::extractTo(const std::map<std::string, std::string>& targetsMap) const {
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

            std::vector<char> ResourcesExtractor::extract(const std::string& path) const {
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
            ResourcesExtractor::extract(const std::vector<std::string>& paths) const {
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
