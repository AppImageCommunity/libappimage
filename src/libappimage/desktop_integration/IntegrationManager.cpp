// system
#include <sstream>

// libraries
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <XdgUtils/BaseDir/BaseDir.h>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include "appimage/appimage.h"
#include <appimage/desktop_integration/IntegrationManager.h>
#include <appimage/desktop_integration/Exceptions.h>
#include "integrator/Integrator.h"
#include "integrator/ResourcesExtractor.h"
#include "utils/HashLib.h"

#ifdef LIBAPPIMAGE_THUMBNAILER
#include "thumbnailer/Thumbnailer.h"
#endif

namespace bf = boost::filesystem;

namespace appimage {
    namespace desktop_integration {
        struct IntegrationManager::Priv {
            bf::path xdgDataHome;

#ifdef LIBAPPIMAGE_THUMBNAILER
            thumbnailer::Thumbnailer thumbnailer;
#endif

            std::string generateAppImageId(const std::string& appImagePath) {
                // Generate AppImage Id
                std::string md5 = appimage_get_md5(appImagePath.c_str()) ?: "";

                return "appimagekit_" + md5;
            }

            /**
             * Explore <dir> recursively and remove files that contain <hint> in their name.
             * @param dir
             * @param hint
             */
            void removeMatchingFiles(const bf::path& dir, const std::string& hint) {
                try {
                    for (bf::recursive_directory_iterator it(dir), eit; it != eit; ++it) {
                        if (!bf::is_directory(it->path()) && it->path().string().find(hint) != std::string::npos)
                            bf::remove(it->path());
                    }
                } catch (...) {}
            }
        };

        IntegrationManager::IntegrationManager(const std::string& xdgDataHome) : priv(new Priv) {
            if (xdgDataHome.empty())
                priv->xdgDataHome = XdgUtils::BaseDir::XdgDataHome();
            else
                priv->xdgDataHome = xdgDataHome;
        }

        void IntegrationManager::registerAppImage(const std::string& appImagePath) {
            integrator::Integrator i(appImagePath, priv->xdgDataHome.string());
            i.integrate();
        }

        bool IntegrationManager::isARegisteredAppImage(const std::string& appImagePath) {
            // Generate AppImage Id
            const auto& appImageId = priv->generateAppImageId(appImagePath);

            // look for a desktop entry file with the AppImage Id in its name
            bf::path appsPath = priv->xdgDataHome / "applications";

            try {
                for (bf::recursive_directory_iterator it(appsPath), eit; it != eit; ++it) {
                    if (!bf::is_directory(it->path()) && it->path().string().find(appImageId) != std::string::npos)
                        return true;
                }
            } catch (...) {}

            return false;
        }

        bool IntegrationManager::shallAppImageBeRegistered(const std::string& appImagePath) {
            try {
                // Only extract the Destop Entry
                integrator::ResourcesExtractor extractor(appImagePath);
                extractor.setExtractMimeFiles(false);
                extractor.setExtractAppDataFile(false);
                extractor.setExtractIconFiles(false);

                auto resources = extractor.extract();

                if (resources.desktopEntryData.empty())
                    throw DesktopIntegrationError("Missing Desktop Entry");

                // Read Desktop Entry contents
                std::string desktopEntryDataString(resources.desktopEntryData.begin(),
                                                   resources.desktopEntryData.end());
                XdgUtils::DesktopEntry::DesktopEntry entry(desktopEntryDataString);

                auto integrateValue = entry.get("Desktop Entry/X-AppImage-Integrate");
                boost::algorithm::erase_all(integrateValue, " ");
                boost::algorithm::to_lower(integrateValue);

                if (integrateValue == "false")
                    return false;

                auto terminalValue = entry.get("Desktop Entry/Terminal");
                boost::algorithm::erase_all(terminalValue, " ");
                boost::algorithm::to_lower(terminalValue);
                if (terminalValue == "true")
                    return false;


            } catch (const appimage::core::AppImageError& error) {
                throw DesktopIntegrationError("Unable to read the AppImage");
            }


            return true;
        }

        void IntegrationManager::unregisterAppImage(const std::string& appImagePath) {
            // Generate AppImage Id
            const auto appImageId = priv->generateAppImageId(appImagePath);

            // remove files with the
            priv->removeMatchingFiles(priv->xdgDataHome / "applications", appImageId);
            priv->removeMatchingFiles(priv->xdgDataHome / "icons", appImageId);
            priv->removeMatchingFiles(priv->xdgDataHome / "mime/packages", appImageId);
        }

        void IntegrationManager::generateThumbnails(const std::string& appImagePath) {
#ifdef LIBAPPIMAGE_THUMBNAILER
            priv->thumbnailer.create(appImagePath);
#endif
        }

        void IntegrationManager::removeThumbnails(const std::string& appImagePath) {
#ifdef LIBAPPIMAGE_THUMBNAILER
            priv->thumbnailer.remove(appImagePath);
#endif
        }

        IntegrationManager::~IntegrationManager() = default;
    }
}
