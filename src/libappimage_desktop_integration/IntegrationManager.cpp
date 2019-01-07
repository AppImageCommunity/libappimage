// system

// libraries
#include <boost/filesystem.hpp>
#include <XdgUtils/BaseDir/BaseDir.h>

// local
#include <appimage/desktop_integration/IntegrationManager.h>
#include "integrator/Integrator.h"
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

        IntegrationManager::~IntegrationManager() = default;
    }
}
