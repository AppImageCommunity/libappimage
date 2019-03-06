// system
#include <sstream>

// libraries
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>
#include <XdgUtils/BaseDir/BaseDir.h>


// local
#include "utils/Logger.h"
#include "utils/IconHandle.h"
#include "utils/path_utils.h"
#include "Thumbnailer.h"

using namespace appimage::utils;
namespace bf = boost::filesystem;

namespace appimage {
    namespace desktop_integration {
        Thumbnailer::Thumbnailer() : xdgCacheHome(XdgUtils::BaseDir::Home() + "/.cache") {}

        Thumbnailer::Thumbnailer(const std::string& xdgCacheHome) : xdgCacheHome(xdgCacheHome) {
            /* XDG_CACHE_HOME path is required to deploy the thumbnails */
            if (Thumbnailer::xdgCacheHome.empty())
                Thumbnailer::xdgCacheHome = XdgUtils::BaseDir::Home() + "/.cache";
        }

        void Thumbnailer::create(const core::AppImage& appImage) {
            utils::ResourcesExtractor extractor(appImage);

            /* Just the application main icon will be used to generate the thumbnails */
            std::string appIcon = getAppIconName(extractor);

            /* According to the xdg thumbnails spec files should be named after the
             * md5 sum of it's canonical path. */
            std::string canonicalPathMd5 = hashPath(appImage.getPath());

            auto appIconsPaths = extractor.getIconFilePaths(appIcon);

            // Look for the bests icons to to be used while generating the thumbnails
            auto normalIconPath = getIconPath(appIconsPaths, "128x128");
            auto largeIconPath = getIconPath(appIconsPaths, "256x256");

            auto iconsData = extractor.extract(std::vector<std::string>{normalIconPath, largeIconPath});

            generateNormalSizeThumbnail(canonicalPathMd5, iconsData[normalIconPath]);
            generateLargeSizeThumbnail(canonicalPathMd5, iconsData[largeIconPath]);
        }

        void Thumbnailer::remove(const std::string& appImagePath) {
            /* Every resource file related with this appimage has the md5 sum of the appimage canonical
             * path in its name, we are going to use this to recreate the file names */
            std::string canonicalPathMd5 = hashPath(appImagePath);
            bf::path normalThumbnailPath = getNormalThumbnailPath(canonicalPathMd5);
            bf::path largeThumbnailPath = getLargeThumbnailPath(canonicalPathMd5);

            bf::remove(normalThumbnailPath);
            bf::remove(largeThumbnailPath);
        }

        void Thumbnailer::generateNormalSizeThumbnail(const std::string& canonicalPathMd5,
                                                      std::vector<char>& normalIconData) const {

            bf::path normalThumbnailPath = getNormalThumbnailPath(canonicalPathMd5);

            /* It required that the folders were the thumbnails will be deployed to exists */
            bf::create_directories(normalThumbnailPath.parent_path());

            try {
                IconHandle iconHandle(normalIconData);
                /* large thumbnails are 128x128, let's be sure of it*/
                iconHandle.setSize(128);
                iconHandle.save(normalThumbnailPath.string(), "png");
                return;
            } catch (const IconHandleError&) {
                /* we fail to resize the icon because it's in an unknown format or some other reason
                 * we just have left to write it down unchanged and hope for the best. */
                Logger::warning("Unable to resize the application icon into a 128x128 image, "
                                "it will be written unchanged.");
            }

            // It wasn't possible to generate a thumbnail, therefore the the icon will be written unchanged
            std::ofstream out(normalThumbnailPath.string());
            out.write(normalIconData.data(), normalIconData.size());
        }

        void Thumbnailer::generateLargeSizeThumbnail(const std::string& canonicalPathMd5,
                                                     std::vector<char>& largeIconData) const {
            bf::path largeThumbnailPath = getLargeThumbnailPath(canonicalPathMd5);

            /* It required that the folders were the thumbnails will be deployed to exists */
            bf::create_directories(largeThumbnailPath.parent_path());

            try {
                IconHandle iconHandle(largeIconData);
                /* large thumbnails are 256x256, let's be sure of it*/
                iconHandle.setSize(256);

                /* thumbnails are always png */
                iconHandle.save(largeThumbnailPath.string(), "png");
                return;
            } catch (const IconHandleError&) {
                /* we fail to resize the icon because it's in an unknown format or some other reason
                 * we just have left to write it down unchanged and hope for the best. */
                Logger::warning("Unable to resize the application icon into a 256x256 image, "
                                "it will be written unchanged.");
            }

            // It wasn't possible to generate a thumbnail, therefore the the icon will be written unchanged
            std::ofstream out(largeThumbnailPath.string());
            out.write(largeIconData.data(), largeIconData.size());
            out.close();
        }


        bf::path Thumbnailer::getNormalThumbnailPath(const std::string& canonicalPathMd5) const {
            bf::path xdgCacheHomePath(xdgCacheHome);

            bf::path normalThumbnailPath = xdgCacheHomePath / normalThumbnailsPrefix /
                                           (canonicalPathMd5 + thumbnailFileExtension);
            return normalThumbnailPath;
        }

        bf::path Thumbnailer::getLargeThumbnailPath(const std::string& canonicalPathMd5) const {
            bf::path xdgCacheHomePath(xdgCacheHome);

            bf::path largeThumbnailPath = xdgCacheHomePath / largeThumbnailPrefix /
                                          (canonicalPathMd5 + thumbnailFileExtension);
            return largeThumbnailPath;
        }

        std::string Thumbnailer::getIconPath(std::vector<std::string> appIcons, const std::string& size) {
            /* look for an icon with the size required or an scalable one. It will be resized latter */
            for (const auto& itr: appIcons) {
                if (itr.find(size) != std::string::npos ||
                    itr.find("/scalable/") != std::string::npos) {
                    return itr;
                }
            }

            // Fallback to ".DirIcon"
            return ".DirIcon";

        }

        std::string Thumbnailer::getAppIconName(const ResourcesExtractor& resourcesExtractor) const {
            auto desktopEntryPath = resourcesExtractor.getDesktopEntryPath();
            auto desktopEntryData = resourcesExtractor.extractText(desktopEntryPath);

            XdgUtils::DesktopEntry::DesktopEntry entry(desktopEntryData);
            return entry.get("Desktop Entry/Icon");
        }

        Thumbnailer::~Thumbnailer() = default;
    }
}
