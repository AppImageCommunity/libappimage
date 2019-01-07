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

            /**
             * @brief Register an AppImage in the system
             *
             * Extract the application main desktop entry, icons and mime type packages. Modifies their content to
             * properly match the AppImage file location and deploy them into the use XDG_DATA_HOME appending a
             * prefix to each file. Such prefix is composed as "<vendor id>_<appimage_path_md5>_<old_file_name>"
             *
             * @param appImagePath
             */
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

            /**
             * @brief Check whether the author of an AppImage doesn't want it to be integrated.
             *
             * An AppImage is considered that shall not be integrated if fulfill any of the following conditions:
             *  - The AppImage's desktop file has set X-AppImage-Integrate=false.
             *  - The AppImage's desktop file has set Terminal=true.
             *
             * @param appImagePath
             * @return true if any of the conditions are meet, false otherwise
             */
            bool shallAppImageBeRegistered(const std::string& appImagePath);

        private:
            struct Priv;
            std::unique_ptr<Priv> priv;   // opaque pointer
        };
    }
}
