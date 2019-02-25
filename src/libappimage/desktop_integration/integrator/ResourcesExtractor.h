#pragma once

// system
#include <string>

// libraries
#include <appimage/core/AppImage.h>

// local
#include "PayloadEntriesCache.h"
#include "DesktopIntegrationResources.h"

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            /**
             * Extract the resources (files) required to integrate an AppImage into the desktop environment
             * in an efficient way.
             *
             * Using the `PayloadIterator::read` method on symlinks is not reliable as it's not supported on
             * AppImages of type 1 (blame on `libarchive`). To overcome this limitation two iterations over the
             * AppImage will be performed. One to resolve all the links entries and other to actually extract
             * the resources.
             */
            class ResourcesExtractor {
            public:
                explicit ResourcesExtractor(const core::AppImage& appImage);

                /**
                 * @brief Extract the desktop integration resources in a reliable way.
                 *
                 * @return extracted integration resources
                 * @throw PayloadIteratorError if there is a links cycle, e.g. A links to A who links to C who links to A
                 */
                DesktopIntegrationResources extract();

                void setExtractDesktopFile(bool extractDesktopFile);

                void setExtractIconFiles(bool extractIconFiles);

                void setExtractAppDataFile(bool extractAppDataFile);

                void setExtractMimeFiles(bool extractMimeFiles);

                XdgUtils::DesktopEntry::DesktopEntry extractDesktopEntry();

                /**
                 * @brief Read an entry into memory, if the entry is a link it will be resolved.
                 * @return entry data
                 * @throw PayloadIteratorError if the entry doesn't exists
                 */
                std::vector<char> extractFile(const std::string& path);

                /**
                 * @brief Read each entry into memory, if the entry is a link it will be resolved.
                 * @return entries data
                 * @throw PayloadIteratorError if some entry doesn't exists
                 */
                std::map<std::string, std::vector<char>> extractFiles(const std::vector<std::string>& paths);

                /**
                 * Icons are expected to be located in "usr/share/icons/" according to the FreeDesktop
                 * Icon Theme Specification. This method look for entries in that path whose file name
                 * matches to the iconName
                 *
                 * @param iconName
                 * @return list of the icon entries paths
                 */
                std::vector<std::string> getIconFilePaths(const std::string& iconName);

                /**
                 * Mime-Type packages are xml files located usr/share/mime/packages according to the
                 * Shared MIME-info Database specification.
                 *
                 * @param iconName
                 * @return Mime-Type packages entries paths
                 */
                std::vector<std::string> getMimeTypePackagesPaths();

                /**
                 * Extract entries listed in 'first' member of the <targetsMap> iterator to the 'second' member
                 * of the <targetsMap> iterator. Will resolve links to regular files.
                 *
                 * @param targetsMap
                 */
                void extractEntriesTo(const std::map<std::string, std::string>& targetsMap);

            private:
                bool extractDesktopFile = false;
                bool extractIconFiles = false;
                bool extractAppDataFile = false;
                bool extractMimeFiles = false;

                core::AppImage appImage;
                PayloadEntriesCache entriesCache;

                bool isIconFile(const std::string& fileName) const;

                bool isMainDesktopFile(const std::string& fileName) const;

                bool isAppDataFile(const std::string& filePath) const;

                bool isMimeFile(const std::string& filePath) const;

                std::vector<char> readWholeFile(std::istream& istream);

                /**
                 * Resolve the final paths of the resource entries.
                 *
                 * Resource entries can be links to other entries therefore several iterations will be required
                 * to extract a regular file for the given resource. This method uses the <entriesCache> to
                 * resolve the final paths of the regular entries to allow their extraction in the next iteration.
                 *
                 * @param resources
                 */
                void resolveResourcesFinalPaths(DesktopIntegrationResources& resources, PayloadEntriesCache cache);

                /**
                 * Read the data of the entries listed in <resources>
                 *
                 * @param resources
                 */
                void readResourceFiles(DesktopIntegrationResources& resources);
            };
        }
    }
}
