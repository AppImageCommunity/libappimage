// system
#include <sstream>

// libraries
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>
#include <XdgUtils/BaseDir/BaseDir.h>


// local
#include "integrator/ResourcesExtractor.h"
#include "utils/IconHandle.h"
#include "utils/path_utils.h"
#include "Thumbnailer.h"

using namespace appimage::utils;
namespace bf = boost::filesystem;

namespace appimage {
    namespace desktop_integration {
        Thumbnailer::Thumbnailer() : xdgCacheHome(XdgUtils::BaseDir::Home() + "/.cache"),
                                     logger("Thumbnailer", std::clog) {}

        Thumbnailer::Thumbnailer(const std::string& xdgCacheHome) : xdgCacheHome(xdgCacheHome),
                                                                    logger("Thumbnailer", std::clog) {
            /* XDG_CACHE_HOME path is required to deploy the thumbnails */
            if (Thumbnailer::xdgCacheHome.empty())
                Thumbnailer::xdgCacheHome = XdgUtils::BaseDir::Home() + "/.cache";
        }

        void Thumbnailer::create(const core::AppImage& appImage) {
            auto resources = extractResources(appImage);
            /* Just the application main icon will be used to generate the thumbnails */
            std::string appIcon = getAppIconName(resources);

            /* According to the xdg thumbnails spec files should be named after the
             * md5 sum of it's canonical path. */
            std::string canonicalPathMd5 = getCanonicalPathMd5(appImage.getPath());

            /* Get from the resources the icon that will be easily resized to 128x128  */
            auto normalIconData = getIconData(resources, appIcon, "128x128");
            generateNormalSizeThumbnail(canonicalPathMd5, normalIconData);

            /* Get from the resources the icon that will be easily resized to 256x256  */
            auto largeIconData = getIconData(resources, appIcon, "256x256");
            generateLargeSizeThumbnail(canonicalPathMd5, largeIconData);
        }

        void Thumbnailer::remove(const core::AppImage& appImage) {
            /* Every resource file related with this appimage has the md5 sum of the appimage canonical
             * path in its name, we are going to use this to recreate the file names */
            std::string canonicalPathMd5 = getCanonicalPathMd5(appImage.getPath());
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
                std::vector<uint8_t> iconData{normalIconData.begin(), normalIconData.end()};
                IconHandle iconHandle(iconData);
                /* large thumbnails are 128x128, let's be sure of it*/
                iconHandle.setSize(128);
                iconHandle.save(normalThumbnailPath.string(), "png");
                return;
            } catch (const IconHandleError&) {
                /* we fail to resize the icon because it's in an unknown format or some other reason
                 * we just have left to write it down as it is and hope for the best. */
                logger.warning() << "Unable to resize the application icon into a 128x128 image, it will be "
                                    "written as it's." << std::endl;
            }

            // It wasn't possible to generate a thumbnail, therefore the the icon will be written as it's
            bf::ofstream out(normalThumbnailPath);
            out.write(normalIconData.data(), normalIconData.size());
        }

        void Thumbnailer::generateLargeSizeThumbnail(const std::string& canonicalPathMd5,
                                                     std::vector<char>& largeIconData) const {
            bf::path largeThumbnailPath = getLargeThumbnailPath(canonicalPathMd5);

            /* It required that the folders were the thumbnails will be deployed to exists */
            bf::create_directories(largeThumbnailPath.parent_path());

            try {
                std::vector<uint8_t> iconData{largeIconData.begin(), largeIconData.end()};
                IconHandle iconHandle(iconData);
                /* large thumbnails are 256x256, let's be sure of it*/
                iconHandle.setSize(256);

                /* thumbnails are always png */
                iconHandle.save(largeThumbnailPath.string(), "png");
                return;
            } catch (const IconHandleError&) {
                /* we fail to resize the icon because it's in an unknown format or some other reason
                 * we just have left to write it down as it is and hope for the best. */
                logger.warning() << "Unable to resize the application icon into a 256x256 image, it will be "
                                    "written as it's." << std::endl;
            }

            // It wasn't possible to generate a thumbnail, therefore the the icon will be written as it's
            bf::ofstream out(largeThumbnailPath);
            out.write(largeIconData.data(), largeIconData.size());
            out.close();
        }


        std::string Thumbnailer::getCanonicalPathMd5(const std::string& appImagePath) const {
            auto canonicalAppImagePath = bf::weakly_canonical(appImagePath).string();
            std::string canonicalPathMd5 = utils::hashPath(canonicalAppImagePath);

            return canonicalPathMd5;
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

        std::vector<char> Thumbnailer::getIconData(const DesktopIntegrationResources& resources,
                                                   const std::string& appIcon, const std::string& iconSize) {
            std::vector<char> iconData;

            /* look for a perfect match */
            for (const auto& itr: resources.icons) {
                if (itr.first.find(appIcon) != std::string::npos) {
                    if (itr.first.find(iconSize) != std::string::npos)
                        iconData = itr.second;
                }
            }

            /* .DirIcon if there are no perfect matches. Provably we will have to resize it latter */
            auto dirIconItr = resources.icons.find(".DirIcon");
            if (iconData.empty() && dirIconItr != resources.icons.end())
                iconData = dirIconItr->second;

            return iconData;
        }

        std::string Thumbnailer::getAppIconName(const DesktopIntegrationResources& resources) const {
            std::string appIcon;

            if (!resources.desktopEntryData.empty()) {
                std::string desktopEntryData(resources.desktopEntryData.begin(),
                                             resources.desktopEntryData.end());
                XdgUtils::DesktopEntry::DesktopEntry entry(desktopEntryData);
                appIcon = entry.get("Desktop Entry/Icon");
            }
            return appIcon;
        }

        DesktopIntegrationResources Thumbnailer::extractResources(const core::AppImage& appImage) const {
            /* We only need the Desktop Entry and the icons */
            integrator::ResourcesExtractor extractor(appImage);
            extractor.setExtractAppDataFile(false);
            extractor.setExtractMimeFiles(false);

            auto resources = extractor.extract();
            return resources;
        }

        Thumbnailer::~Thumbnailer() = default;
    }
}
