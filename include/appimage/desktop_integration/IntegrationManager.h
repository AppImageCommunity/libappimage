#pragma once

// system
#include <string>
#include <memory>
#include <iostream>

// local
#include <appimage/desktop_integration/exceptions.h>
#include <appimage/core/AppImage.h>
#include <appimage/config.h>

namespace appimage {
    namespace desktop_integration {
        class IntegrationManager {
        public:
            /**
             * Instantiate an Integration manager that will use as XDG_DATA_HOME the one pointed by the system
             * configuration.
             */
            explicit IntegrationManager();

            /**
             * Instantiate an Integration manager that will use as XDG_DATA_HOME the one pointed by the <xdgDataHome>
             */
            explicit IntegrationManager(const std::string& xdgDataHome);

            /**
            * Creates an IntegrationManager instance from <other> IntegrationManager
            * @param other
            */
            IntegrationManager(const IntegrationManager& other);

            /**
             * Copy the <other> instance data into the current one.
             * @param other
             * @return
             */
            IntegrationManager& operator=(const IntegrationManager& other);

            virtual ~IntegrationManager();

            /**
             * @brief Register an AppImage in the system
             *
             * Extract the application main desktop entry, icons and mime type packages. Modifies their content to
             * properly match the AppImage file location and deploy them into the use XDG_DATA_HOME appending a
             * prefix to each file. Such prefix is composed as "<vendor id>_<appimage_path_md5>_<old_file_name>"
             *
             * @param appImage
             */
            void registerAppImage(const core::AppImage& appImage) const;

            /**
             * @brief Register an AppImage in the system adding custom desktop entry actions
             *
             * Extract the application main desktop entry, icons and mime type packages. Modifies their content to
             * properly match the AppImage file location and deploy them into the use XDG_DATA_HOME appending a
             * prefix to each file. Such prefix is composed as "<vendor id>_<appimage_path_md5>_<old_file_name>"
             *
             * The desktop entry actions must follow the specification for additional application actions at https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s11.html.
             * The map key should be the action identifier and the value the action fields in a plain string i.e.:
             *
             *  ```
             *  std::map<std::string, std::string> additionalApplicationActions = {{"Remove",
             *                                                    "[Desktop Action Remove]\n"
             *                                                    "Name=\"Remove application\"\n"
             *                                                    "Icon=remove\n"
             *                                                    "Exec=remove-appimage-helper /path/to/the/AppImage\n"}};
             *```
             * @param appImage
             * @param additionalApplicationActions desktop entry actions to be added.
             */
            void registerAppImage(const core::AppImage& appImage, std::map<std::string, std::string> additionalApplicationActions) const;

            /**
             * @brief Unregister an AppImage in the system
             *
             * Remove all files created by the registerAppImage function. The files are identified by matching the
             * AppImageId contained in their names. The Id is made from the MD5 checksum of the <appImagePath>.
             * @param appImagePath
             */
            void unregisterAppImage(const std::string& appImagePath) const;

            /**
             * @brief Check whether the AppImage pointed by <appImagePath> has been registered in the system.
             *
             * Explore XDG_DATA_HOME/applications looking for Destkop Entries files with a file name that matches
             * the current AppImage Id (MD5 checksum of the <appImagePath>)
             *
             * @param appImagePath
             * @return true if the AppImage is registered, false otherwise.
             */
            bool isARegisteredAppImage(const std::string& appImagePath) const;

            /**
             * @brief Check whether the author of an AppImage doesn't want it to be integrated.
             *
             * An AppImage is considered that shall not be integrated if fulfill any of the following conditions:
             *  - The AppImage's desktop file has set X-AppImage-Integrate=false.
             *  - The AppImage's desktop file has set Terminal=true.
             *
             * @param appImage
             * @return false if any of the conditions are meet, true otherwise
             */
            bool shallAppImageBeRegistered(const core::AppImage& appImage) const;

#ifdef LIBAPPIMAGE_THUMBNAILER_ENABLED
            /**
             * @brief Generate thumbnails according to the FreeDesktop Thumbnail Managing Standard
             * See: https://specifications.freedesktop.org/thumbnail-spec/0.8.0/index.html
             * @param appImage
             */
            void generateThumbnails(const core::AppImage& appImage) const;

            /**
             * @brief Remove thumbnails according to the FreeDesktop Thumbnail Managing Standard
             * See: https://specifications.freedesktop.org/thumbnail-spec/0.8.0/index.html
             * @param appImagePath
             */
            void removeThumbnails(const std::string& appImagePath) const;

#endif

        private:
            class Private;
            std::shared_ptr<Private> d;   // opaque pointer
        };
    }
}
