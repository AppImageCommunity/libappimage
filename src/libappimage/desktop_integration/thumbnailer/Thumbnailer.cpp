// system
#include <sstream>

// libraries
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>
#include <XdgUtils/BaseDir/BaseDir.h>

// Required to avoid libX11 use at CImg
#define cimg_display 0
#define cimg_verbosity 0

#include <CImg.h>


// local
#include "appimage/appimage.h"
#include "integrator/ResourcesExtractor.h"
#include "SvgThumbnailer.h"
#include "Thumbnailer.h"

using namespace cimg_library;
namespace bf = boost::filesystem;

namespace appimage {
    namespace desktop_integration {
        namespace thumbnailer {
            Thumbnailer::Thumbnailer(const std::string& xdgCacheHome) : xdgCacheHome(xdgCacheHome) {
                if (Thumbnailer::xdgCacheHome.empty())
                    Thumbnailer::xdgCacheHome = XdgUtils::BaseDir::Home() + "/.cache";

                // load Svg Thumbnailer if possible
                try {
                    svgThumbnailer.reset(new SvgThumbnailer);
                } catch (...) {
                    svgThumbnailer.reset(nullptr);
                }

            }

            void Thumbnailer::create(const std::string& appImagePath) {
                auto resources = extractResources(appImagePath);
                std::string appIcon = getAppIconName(resources);

                std::string canonicalPathMd5 = getCanonicalPathMd5(appImagePath);

                std::vector<char> normalIconData = getIconData(resources, appIcon, "128x128");
                generateNormalSizeThumbnail(canonicalPathMd5, normalIconData);

                std::vector<char> largeIconData = getIconData(resources, appIcon, "256x256");
                generateLargeSizeThumbnail(canonicalPathMd5, largeIconData);
            }

            void Thumbnailer::generateNormalSizeThumbnail(const std::string& canonicalPathMd5,
                                                          std::vector<char>& normalIconData) const {

                bf::path normalThumbnailPath = getNormalThumbnailPath(canonicalPathMd5);
                bf::create_directories(normalThumbnailPath.parent_path());

                // try using CImg
                try {
                    CImg<float> normalIcon(normalIconData.data());
                    normalIcon.resize(128, 128);
                    normalIcon.save_png(normalThumbnailPath.string().c_str());

                    return;
                } catch (...) {}

                // CImg thumbnail generation failed is provably because it's a SVG file
                if (svgThumbnailer) {
                    try {
                        auto result = svgThumbnailer->create(normalIconData, 128);
                        bf::ofstream out(normalThumbnailPath);
                        out.write(result.data(), result.size());

                        return;
                    } catch (...) {}
                }

                // It wasn't possible to generate a thumbnail, therefore the the icon will be written as it's
                bf::ofstream out(normalThumbnailPath);
                out.write(normalIconData.data(), normalIconData.size());
            }

            void Thumbnailer::generateLargeSizeThumbnail(const std::string& canonicalPathMd5,
                                                         std::vector<char>& largeIconData) const {
                bf::path largeThumbnailPath = getLargeThumbnailPath(canonicalPathMd5);
                bf::create_directories(largeThumbnailPath.parent_path());

                // try using CImg
                try {
                    CImg<float> normalIcon(largeIconData.data());
                    normalIcon.resize(256, 256);
                    normalIcon.save_png(largeThumbnailPath.string().c_str());

                    return;
                } catch (...) {}

                // CImg thumbnail generation failed is provably because it's a SVG file
                if (svgThumbnailer) {
                    try {
                        auto result = svgThumbnailer->create(largeIconData, 256);
                        bf::ofstream out(largeThumbnailPath);
                        out.write(result.data(), result.size());

                        return;
                    } catch (...) {}
                }


                // It wasn't possible to generate a thumbnail, therefore the the icon will be written as it's
                bf::ofstream out(largeThumbnailPath);
                out.write(largeIconData.data(), largeIconData.size());
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

            DesktopIntegrationResources Thumbnailer::extractResources(const std::string& appImagePath) const {
                integrator::ResourcesExtractor extractor(appImagePath);
                extractor.setExtractAppDataFile(false);
                extractor.setExtractMimeFiles(false);

                auto resources = extractor.extract();
                return resources;
            }

            Thumbnailer::~Thumbnailer() = default;
        }
    }
}
