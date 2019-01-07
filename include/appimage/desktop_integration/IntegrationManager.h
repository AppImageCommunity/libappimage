#pragma once

// system
#include <string>
#include <memory>

namespace appimage {
    namespace desktop_integration {
        class IntegrationManager {
        public:
            explicit IntegrationManager(const std::string& xdgDataDir = std::string());

            virtual ~IntegrationManager();

            void registerAppImage(const std::string& appImagePath);

            void unregisterAppImage(const std::string& appImagePath);

            bool isARegisteredAppImage(const std::string& appImagePath);

            bool shallAppImageBeRegistered(const std::string& appImagePath);

        private:
            struct Priv;
            std::unique_ptr<Priv> priv;   // opaque pointer
        };
    }
}
