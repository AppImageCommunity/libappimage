// system
#include <sstream>

// libraries
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>
#include <XdgUtils/BaseDir/BaseDir.h>


// local
#include "appimage/appimage.h"
#include "integrator/ResourcesExtractor.h"
#include "utils/IconHandle.h"
#include "Thumbnailer.h"

using namespace appimage::utils;
namespace bf = boost::filesystem;

namespace appimage {
    namespace desktop_integration {
        Thumbnailer::Thumbnailer(const std::string& xdgCacheHome) : xdgCacheHome(xdgCacheHome) {
            if (Thumbnailer::xdgCacheHome.empty())
                Thumbnailer::xdgCacheHome = XdgUtils::BaseDir::Home() + "/.cache";

        }

        void Thumbnailer::create(const core::AppImage& appImage) {
            auto resources = extractResources(appImage);
            std::string appIcon = getAppIconName(resources);

            std::string canonicalPathMd5 = getCanonicalPathMd5(appImage.getPath());

            std::vector<char> normalIconData = getIconData(resources, appIcon, "128x128");
            generateNormalSizeThumbnail(canonicalPathMd5, normalIconData);

            std::vector<char> largeIconData = getIconData(resources, appIcon, "256x256");
            generateLargeSizeThumbnail(canonicalPathMd5, largeIconData);
        }

        void Thumbnailer::remove(const core::AppImage& appImage) {
            std::string canonicalPathMd5 = getCanonicalPathMd5(appImage.getPath());
            bf::path normalThumbnailPath = getNormalThumbnailPath(canonicalPathMd5);
            bf::path largeThumbnailPath = getLargeThumbnailPath(canonicalPathMd5);

            bf::remove(normalThumbnailPath);
            bf::remove(largeThumbnailPath);
        }

        void Thumbnailer::generateNormalSizeThumbnail(const std::string& canonicalPathMd5,
                                                      std::vector<char>& normalIconData) const {

            bf::path normalThumbnailPath = getNormalThumbnailPath(canonicalPathMd5);
            bf::create_directories(normalThumbnailPath.parent_path());

            try {
                std::vector<uint8_t> iconData{normalIconData.begin(), normalIconData.end()};
                IconHandle iconHandle(iconData);
                iconHandle.setSize(128);
                iconHandle.save(normalThumbnailPath.string(), "png");
                return;
            } catch (const IconHandleError &) {}

            // It wasn't possible to generate a thumbnail, therefore the the icon will be written as it's
            bf::ofstream out(normalThumbnailPath);
            out.write(normalIconData.data(), normalIconData.size());
        }

        void Thumbnailer::generateLargeSizeThumbnail(const std::string& canonicalPathMd5,
                                                     std::vector<char>& largeIconData) const {
            bf::path largeThumbnailPath = getLargeThumbnailPath(canonicalPathMd5);
            bf::create_directories(largeThumbnailPath.parent_path());

            try {
                std::vector<uint8_t> iconData{largeIconData.begin(), largeIconData.end()};
                IconHandle iconHandle(iconData);
                iconHandle.setSize(256);
                iconHandle.save(largeThumbnailPath.string(), "png");
                return;
            } catch (const IconHandleError &) {}

            // It wasn't possible to generate a thumbnail, therefore the the icon will be written as it's
            bf::ofstream out(largeThumbnailPath);
            out.write(largeIconData.data(), largeIconData.size());
            out.close();
        }

        std::string Thumbnailer::getCanonicalPathMd5(const std::string& appImagePath) const {
            auto canonicalAppImagePath = boost::filesystem::weakly_canonical(appImagePath).string();
            auto md5Raw = appimage_get_md5(canonicalAppImagePath.c_str());
            std::string canonicalPathMd5 = md5Raw ?: "";

            free(md5Raw);
            return canonicalPathMd5;
        }


        boost::filesystem::path Thumbnailer::getNormalThumbnailPath(const std::string& canonicalPathMd5) const {
            boost::filesystem::path xdgCacheHomePath(xdgCacheHome);

            boost::filesystem::path normalThumbnailPath =
                xdgCacheHomePath / "thumbnails/normal" / (canonicalPathMd5 + ".png");
            return normalThumbnailPath;
        }

        boost::filesystem::path Thumbnailer::getLargeThumbnailPath(const std::string& canonicalPathMd5) const {
            boost::filesystem::path xdgCacheHomePath(xdgCacheHome);

            boost::filesystem::path largetThumbnailPath =
                xdgCacheHomePath / "thumbnails/large" / (canonicalPathMd5 + ".png");
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
            integrator::ResourcesExtractor extractor(appImage);
            extractor.setExtractAppDataFile(false);
            extractor.setExtractMimeFiles(false);

            auto resources = extractor.extract();
            return resources;
        }

        Thumbnailer::~Thumbnailer() = default;
    }
}
