#pragma once

// system
#include <set>
#include <string>

// local
#include <XdgUtils/DesktopEntry/DesktopEntry.h>

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            /**
             * @brief Edit a Desktop Entry from an AppImage to deploy it into the system.
             *
             * Taking a <desktopEntry> as input this class allows to reset the 'Exec', and 'Icon' entries to new values.
             */
            class DesktopEntryEditor {
            public:
                /**
                 * @param appImagePath
                 */
                void setAppImagePath(const std::string& appImagePath);

                /**
                 * @param appImageVersion
                 */
                void setAppImageVersion(const std::string& appImageVersion);

                /**
                 * @param vendorPrefix usually the AppImagePath md5 sum
                 */
                void setVendorPrefix(const std::string& vendorPrefix);

                /**
                 * Set the uuid that will identify the deployed AppImage resources.
                 * Usually this value is a md5 sum of the AppImage path.
                 * @param uuid
                 */
                void setIdentifier(const std::string& uuid);

                /**
                 * Modifies the Desktop Entry according to the set parameters.
                 * @param desktopEntry
                 */
                void edit(XdgUtils::DesktopEntry::DesktopEntry& desktopEntry);

            private:
                std::string identifier;
                std::string vendorPrefix;
                std::string appImagePath;
                std::string appImageVersion;

                /**
                 * Set Exec and TryExec entries in the 'Desktop Entry' and 'Desktop Action' groups pointing to the
                 * <appImagePath>.
                 */
                void setExecPaths(XdgUtils::DesktopEntry::DesktopEntry& entry);

                /**
                 * Set Icon entries in the 'Desktop Entry' and 'Desktop Action' groups pointing to the new icon names.
                 * The new icon names have the following structure <vendorPrefix>_<uuid>_<oldIconName>
                 */
                void setIcons(XdgUtils::DesktopEntry::DesktopEntry& entry);

                /**
                 * Append the <appImageVersion> to the Name entries in the 'Desktop Entry' group.
                 * The new names will have the following structure "<oldIconName> (<appImageVersion>)"
                 *
                 * If the appImageVersion is not set the value from "Desktop Entry/X-AppImage-Version" will be used instead.
                 * If none of both options are valid the names will remain unchanged.
                 */
                void appendVersionToName(XdgUtils::DesktopEntry::DesktopEntry& entry);
            };

        }
    }
}
