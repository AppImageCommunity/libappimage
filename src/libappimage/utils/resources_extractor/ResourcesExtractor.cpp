// system
#include <map>
#include <set>
#include <fstream>

// libraries
#include <boost/filesystem.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include <appimage/core/PayloadEntryType.h>
#include <appimage/desktop_integration/exceptions.h>
#include "PayloadEntriesCache.h"
#include <appimage/utils/ResourcesExtractor.h>


using namespace XdgUtils::DesktopEntry;
using namespace appimage::core;
namespace bf = boost::filesystem;

namespace appimage {
    namespace utils {
        class ResourcesExtractor::Priv {
        public:
            explicit Priv(const AppImage& appImage) : appImage(appImage), entriesCache(appImage) {}


            core::AppImage appImage;

            PayloadEntriesCache entriesCache;

            bool isFile(const std::string& fileName) const {
                return appimage::core::PayloadEntryType::REGULAR == entriesCache.getEntryType(fileName) ||
                       appimage::core::PayloadEntryType::LINK == entriesCache.getEntryType(fileName);
            }

            bool isIconFile(const std::string& fileName) const {
                return (fileName.find("usr/share/icons") != std::string::npos) && isFile(fileName);
            }

            bool isMainDesktopFile(const std::string& fileName) const {
                return fileName.find(".desktop") != std::string::npos &&
                       fileName.find('/') == std::string::npos && isFile(fileName);
            }

            bool isMimeFile(const std::string& fileName) const {
                return fileName.find("usr/share/mime/packages") != std::string::npos &&
                       fileName.find(".xml") != std::string::npos && isFile(fileName);
            }

            std::vector<char> readDataFile(std::istream& istream) const {
                return {std::istreambuf_iterator<char>(istream), std::istreambuf_iterator<char>()};
            }

            std::string readTextFile(std::istream& istream) const {
                return {std::istreambuf_iterator<char>(istream), std::istreambuf_iterator<char>()};
            }
        };

        ResourcesExtractor::ResourcesExtractor(const core::AppImage& appImage) : d(new Priv(appImage)) {}


        std::vector<std::string> ResourcesExtractor::getIconFilePaths(const std::string& iconName) const {
            std::vector<std::string> filePaths;

            for (const auto& filePath: d->entriesCache.getEntriesPaths()) {
                if (d->isIconFile(filePath) &&
                    filePath.find(iconName) != std::string::npos) {
                    filePaths.emplace_back(filePath);
                }
            }

            return filePaths;
        }

        std::vector<std::string> ResourcesExtractor::getMimeTypePackagesPaths() const {
            std::vector<std::string> filePaths;

            for (const auto& filePath: d->entriesCache.getEntriesPaths()) {
                if (d->isMimeFile(filePath))
                    filePaths.emplace_back(filePath);
            }

            return filePaths;
        }

        void ResourcesExtractor::extractTo(const std::map<std::string, std::string>& targetsMap) const {
            // Resolve links to ensure proper extraction
            std::map<std::string, std::string> realTargetsMap;
            for (const auto& target: targetsMap) {
                if (d->entriesCache.getEntryType(target.first) == PayloadEntryType::LINK) {
                    const std::string& realTarget = d->entriesCache.getEntryLinkTarget(target.first);
                    realTargetsMap[realTarget] = target.second;
                } else {
                    realTargetsMap.insert(target);
                }
            }


            for (auto fileItr = d->appImage.files(); fileItr != fileItr.end(); ++fileItr) {
                const auto deployPathMapping = realTargetsMap.find(fileItr.path());
                if (deployPathMapping != realTargetsMap.end()) {
                    bf::path deployPath(deployPathMapping->second);

                    // create parent dirs
                    const auto parentDirPath = deployPath.parent_path();
                    bf::create_directories(parentDirPath);

                    // write file contents
                    std::ofstream file(deployPath.string());
                    file << fileItr.read().rdbuf();

                    file.close();
                }
            }

        }

        std::vector<char> ResourcesExtractor::extract(const std::string& path) const {
            // Resolve any link before extracting the file
            auto regularEntryPath = path;
            if (d->entriesCache.getEntryType(path) == PayloadEntryType::LINK)
                regularEntryPath = d->entriesCache.getEntryLinkTarget(path);

            for (auto fileItr = d->appImage.files(); fileItr != fileItr.end(); ++fileItr) {
                if (fileItr.path() == regularEntryPath)
                    return d->readDataFile(fileItr.read());
            }

            throw core::PayloadIteratorError("Entry doesn't exists: " + path);
        }

        std::map<std::string, std::vector<char>>
        ResourcesExtractor::extract(const std::vector<std::string>& paths) const {
            // Resolve any link before extracting the files and keep a reference to the original path
            std::map<std::string, std::string> reverseLinks;
            for (const auto& path: paths)
                if (d->entriesCache.getEntryType(path) == PayloadEntryType::LINK)
                    reverseLinks[d->entriesCache.getEntryLinkTarget(path)] = path;
                else
                    reverseLinks[path] = path;

            std::map<std::string, std::vector<char>> result;
            for (auto fileItr = d->appImage.files(); fileItr != fileItr.end(); ++fileItr) {
                auto itr = reverseLinks.find(fileItr.path());

                // extract the file data and store it using the original path
                if (itr != reverseLinks.end())
                    result[itr->second] = d->readDataFile(fileItr.read());
            }

            return result;

        }

        std::string ResourcesExtractor::extractText(const std::string& path) const {
            // Resolve any link before extracting the file
            auto regularEntryPath = path;
            if (d->entriesCache.getEntryType(path) == PayloadEntryType::LINK)
                regularEntryPath = d->entriesCache.getEntryLinkTarget(path);

            for (auto fileItr = d->appImage.files(); fileItr != fileItr.end(); ++fileItr) {
                if (fileItr.path() == regularEntryPath)
                    return d->readTextFile(fileItr.read());
            }

            throw core::PayloadIteratorError("Entry doesn't exists: " + path);
        }

        std::string ResourcesExtractor::getDesktopEntryPath() const {
            for (auto fileItr = d->appImage.files(); fileItr != fileItr.end(); ++fileItr)
                if (d->isMainDesktopFile(fileItr.path()))
                    return fileItr.path();

            throw AppImageError("Missing Desktop Entry");
        }

    }
}
