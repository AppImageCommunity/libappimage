#pragma once

// system
#include <set>
#include <string>

// local
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

namespace appimage {
    namespace desktop_integration {
        /**
         * @brief Edit a Desktop Entry from an AppImage to deploy it into the system.
         *
         * Taking a <baseDesktopEntry> as input this class allows to reset the 'Exec', and 'Icon' entries to new values.
         */
        class DesktopEntryBuilder {
            std::string identifier;
            std::string vendorPrefix;
            std::string appImagePath;
            std::string appImageVersion;
            std::set<std::string> appImageIcons;

            XdgUtils::DesktopEntry::DesktopEntry desktopEntry;

        public:

            void setBaseDesktopFile(std::istream& data);

            void setAppImagePath(const std::string& appImagePath);

            void setAppImageVersion(const std::string& appImageVersion);

            void setVendorPrefix(const std::string& vendorPrefix);

            /**
             * Set the uuid that will identify the deployed AppImage resources.
             * Usually this value is a md5 sum of the AppImage path.
             * @param uuid
             */
            void setIdentifier(const std::string& uuid);

            std::string build();

        private:

            /**
             * Set Exec and TryExec entries in the 'Desktop Entry' and 'Desktop Action' groups pointing to the
             * <appImagePath>.
             */
            void setExecPaths();

            /**
             * Set Icon entries in the 'Desktop Entry' and 'Desktop Action' groups pointing to the new icon names.
             * The new icon names have the following structure <vendorPrefix>_<uuid>_<oldIconName>
             */
            void setIcons();

            /**
             * Append the <appImageVersion> to the Name entries in the 'Desktop Entry' group.
             * The new names will have the following structure "<oldIconName> (<appImageVersion>)"
             *
             * If the appImageVersion is not set the value from "Desktop Entry/X-AppImage-Version" will be used instead.
             * If none of both options are valid the names will remain unchanged.
             */
            void appendVersionToName();
        };

    }
}
