// system
#include <sstream>
#include <filesystem>

// libraries
#include <boost/algorithm/string.hpp>
#include <XdgUtils/BaseDir/BaseDir.h>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include <appimage/desktop_integration/IntegrationManager.h>
#include <appimage/desktop_integration/exceptions.h>
#include <appimage/utils/ResourcesExtractor.h>
#include "integrator/Integrator.h"
#include "utils/hashlib.h"
#include "utils/path_utils.h"
#include "constants.h"

#ifdef LIBAPPIMAGE_THUMBNAILER_ENABLED

#include "Thumbnailer.h"
#endif

namespace appimage {
    namespace desktop_integration {
        class IntegrationManager::Private {
        public:
            std::filesystem::path xdgDataHome;

#ifdef LIBAPPIMAGE_THUMBNAILER_ENABLED
            Thumbnailer thumbnailer;
#endif

            std::string generateAppImageId(const std::string& appImagePath) {
                // Generate AppImage Id
                std::string md5 = utils::hashPath(appImagePath);

                return VENDOR_PREFIX + "_" + md5;
            }

            /**
             * Explore <dir> recursively and remove files that contain <hint> in their name.
             * @param dir
             * @param hint
             */
            void removeMatchingFiles(const std::filesystem::path& dir, const std::string& hint) {
                try {
                    for (std::filesystem::recursive_directory_iterator it(dir), eit; it != eit; ++it) {
                        if (!std::filesystem::is_directory(it->path()) && it->path().string().find(hint) != std::string::npos)
                            std::filesystem::remove(it->path());
                    }
                } catch (const std::filesystem::filesystem_error&) {}
            }
        };

        IntegrationManager::IntegrationManager() : d(new Private) {
            d->xdgDataHome = XdgUtils::BaseDir::XdgDataHome();
        }

        IntegrationManager::IntegrationManager(const std::string& xdgDataHome) : d(new Private) {
            if (xdgDataHome.empty() || !std::filesystem::is_directory(xdgDataHome))
                throw DesktopIntegrationError("Invalid XDG_DATA_HOME: " + xdgDataHome);

            d->xdgDataHome = xdgDataHome;
        }

        void IntegrationManager::registerAppImage(const core::AppImage& appImage) const {
            try {
                integrator::Integrator i(appImage, d->xdgDataHome);
                i.integrate();
            } catch (...) {
                // Remove any file created during the integration process
                unregisterAppImage(appImage.getPath());

                // Rethrow
                throw;
            }
        }

        bool IntegrationManager::isARegisteredAppImage(const std::string& appImagePath) const {
            // Generate AppImage Id
            const auto& appImageId = d->generateAppImageId(appImagePath);

            // look for a desktop entry file with the AppImage Id in its name
            std::filesystem::path appsPath = d->xdgDataHome / "applications";

            try {
                for (std::filesystem::recursive_directory_iterator it(appsPath), eit; it != eit; ++it) {
                    if (!std::filesystem::is_directory(it->path()) && it->path().string().find(appImageId) != std::string::npos)
                        return true;
                }
            } catch (const std::filesystem::filesystem_error&) {}

            return false;
        }

        bool IntegrationManager::shallAppImageBeRegistered(const core::AppImage& appImage) const {
            try {
                utils::ResourcesExtractor extractor(appImage);
                auto desktopEntryPath = extractor.getDesktopEntryPath();
                const auto desktopEntryData = extractor.extractText(desktopEntryPath);

                XdgUtils::DesktopEntry::DesktopEntry entry(desktopEntryData);

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

        void IntegrationManager::unregisterAppImage(const std::string& appImagePath) const {
            // Generate AppImage Id
            const auto appImageId = d->generateAppImageId(appImagePath);

            // remove files with the
            d->removeMatchingFiles(d->xdgDataHome / "applications", appImageId);
            d->removeMatchingFiles(d->xdgDataHome / "icons", appImageId);
            d->removeMatchingFiles(d->xdgDataHome / "mime/packages", appImageId);
        }

#ifdef LIBAPPIMAGE_THUMBNAILER_ENABLED
        void IntegrationManager::generateThumbnails(const core::AppImage& appImage) const {
            d->thumbnailer.create(appImage);
        }

        void IntegrationManager::removeThumbnails(const std::string& appImagePath) const {
            d->thumbnailer.remove(appImagePath);
        }
#endif

        IntegrationManager::IntegrationManager(const IntegrationManager& other) = default;

        IntegrationManager& IntegrationManager::operator=(const IntegrationManager& other) = default;

        IntegrationManager::~IntegrationManager() = default;
    }
}
