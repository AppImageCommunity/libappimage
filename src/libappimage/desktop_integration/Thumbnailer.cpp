// system
#include <sstream>

// libraries
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>
#include <XdgUtils/BaseDir/BaseDir.h>

// Required to avoid libX11 use at CImg
#define cimg_display 0
#include <CImg.h>


// local
#include "appimage/appimage.h"
#include "integrator/ResourcesExtractor.h"
#include "Thumbnailer.h"

using namespace cimg_library;
namespace bf = boost::filesystem;

namespace appimage {
    namespace desktop_integration {
        Thumbnailer::Thumbnailer(const std::string& appImagePath, const std::string& xdgCacheHome)
            : appImagePath(appImagePath), xdgCacheHome(xdgCacheHome) {
            if (Thumbnailer::xdgCacheHome.empty())
                Thumbnailer::xdgCacheHome = XdgUtils::BaseDir::Home() + "/.cache";
        }

        void Thumbnailer::create() {
            auto resources = extractResources();
            std::string appIcon = getAppIconName(resources);


            // find icons of the right size to avoid any transformation
            std::vector<char> normalIconData = getIconData(resources, appIcon, "128x128");
            std::vector<char> largeIconData = getIconData(resources, appIcon, "256x256");

            // set icon right size (if not)
            CImg<float> normalIcon(normalIconData.data());
            normalIcon.resize(128, 128);

            CImg<float> largeIcon(largeIconData.data());
            largeIcon.resize(256, 256);

            std::string canonicalPathMd5 = getCanonicalPathMd5();

            boost::filesystem::path normalThumbnailPath = getNormalThumbnailPath(canonicalPathMd5);
            boost::filesystem::path largeThumbnailPath = getLargeThumbnailPath(canonicalPathMd5);

            normalIcon.save_png(normalThumbnailPath.string().c_str());
            normalIcon.save_png(normalThumbnailPath.string().c_str());
        }

        std::string Thumbnailer::getCanonicalPathMd5() const {
            auto canonicalAppImagePath = boost::filesystem::weakly_canonical(appImagePath).string();
            std::__cxx11::string canonicalPathMd5 = appimage_get_md5(canonicalAppImagePath.c_str()) ?: "";
            return canonicalPathMd5;
        }

        boost::filesystem::path Thumbnailer::getNormalThumbnailPath(std::string canonicalPathMd5) const {
            boost::filesystem::path xdgCacheHomePath(xdgCacheHome);

            boost::filesystem::path normalThumbnailPath =
                xdgCacheHomePath / "thumbnails/normal" / (canonicalPathMd5 + ".png");
            return normalThumbnailPath;
        }

        boost::filesystem::path Thumbnailer::getLargeThumbnailPath(std::string canonicalPathMd5) const {
            boost::filesystem::path xdgCacheHomePath(xdgCacheHome);

            boost::filesystem::path largetThumbnailPath =
                xdgCacheHomePath / "thumbnails/normal" / (canonicalPathMd5 + ".png");
            return largetThumbnailPath;
        }


        std::vector<char> Thumbnailer::getIconData(const DesktopIntegrationResources& resources,
                                                   const std::string& appIcon, const std::string& iconSize) {
            std::vector<char> iconData;

            for (const auto& itr: resources.icons) {
                if (itr.first.find(appIcon) != std::string::npos) {
                    if (itr.first.find(iconSize) != std::string::npos)
                        iconData = itr.second;
                }
            }

            auto dirIconItr = resources.icons.find(".DirIcon");
            if (iconData.empty() && dirIconItr != resources.icons.end())
                iconData = dirIconItr->second;

            return iconData;
        }

        std::string Thumbnailer::getAppIconName(const DesktopIntegrationResources& resources) const {
            std::__cxx11::string appIcon;

            if (!resources.desktopEntryData.empty()) {
                std::__cxx11::string desktopEntryData(resources.desktopEntryData.begin(),
                                                      resources.desktopEntryData.end());
                XdgUtils::DesktopEntry::DesktopEntry entry(desktopEntryData);
                appIcon = entry.get("Desktop Entry/Icon");
            }
            return appIcon;
        }

        DesktopIntegrationResources Thumbnailer::extractResources() const {
            integrator::ResourcesExtractor extractor(appImagePath);
            extractor.setExtractAppDataFile(false);
            extractor.setExtractMimeFiles(false);

            auto resources = extractor.extract();
            return resources;
        }
    }
}
