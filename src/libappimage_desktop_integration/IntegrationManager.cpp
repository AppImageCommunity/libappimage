// system
#include <sstream>

// libraries
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <XdgUtils/BaseDir/BaseDir.h>
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

// local
#include <appimage/desktop_integration/IntegrationManager.h>
#include <appimage/desktop_integration/Exceptions.h>
#include "integrator/Integrator.h"
#include "integrator/ResourcesExtractor.h"
#include "utils/HashLib.h"

namespace bf = boost::filesystem;

namespace appimage {
    namespace desktop_integration {
        struct IntegrationManager::Priv {
            bf::path xdgDataHome;
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
            const auto md5Digest = appimage::utils::HashLib::md5(appImagePath);
            std::string md5 = appimage::utils::HashLib::toHex(md5Digest);

            // look for a desktop entry file with the AppImage Id in its name
            bf::path appsPath = priv->xdgDataHome / "applications";

            try {
                for (bf::recursive_directory_iterator it(appsPath), eit; it != eit; ++it) {
                    if (!bf::is_directory(it->path()) && it->path().string().find(md5) != std::string::npos)
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

        IntegrationManager::~IntegrationManager() = default;
    }
}
