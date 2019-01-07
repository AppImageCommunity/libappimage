// system

// local
#include <appimage/desktop_integration/IntegrationManager.h>
#include "integrator/Integrator.h"

namespace appimage {
    namespace desktop_integration {
        struct IntegrationManager::Priv {
            std::string xdgDataDir;
        };

        void IntegrationManager::registerAppImage(const std::string& appImagePath) {
            integrator::Integrator i(appImagePath, priv->xdgDataDir);
            i.integrate();
        }

        IntegrationManager::IntegrationManager(const std::string& xdgDataDir) : priv(new Priv) {
            priv->xdgDataDir = xdgDataDir;
        }

        IntegrationManager::~IntegrationManager() = default;
    }
}
