#pragma once

// system
#include <string>
#include <memory>

namespace appimage {
    namespace desktop_integration {
        class IntegrationManager {
        public:
            explicit IntegrationManager(const std::string& xdgDataHome = std::string());

            virtual ~IntegrationManager();

            void registerAppImage(const std::string& appImagePath);

            void unregisterAppImage(const std::string& appImagePath);

            /**
             * @brief Check whether the AppImage pointed by <appImagePath> has been registered in the system.
             *
             * Explore XDG_DATA_HOME/applications looking for Destkop Entries files with a file name that matches
             * the current AppImage Id (MD5 checksum of the <appImagePath>)
             *
             * @param appImagePath
             * @return true if the AppImage is registered, false otherwise.
             */
            bool isARegisteredAppImage(const std::string& appImagePath);

            bool shallAppImageBeRegistered(const std::string& appImagePath);

        private:
            struct Priv;
            std::unique_ptr<Priv> priv;   // opaque pointer
        };
    }
}
