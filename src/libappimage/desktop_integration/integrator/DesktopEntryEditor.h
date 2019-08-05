#pragma once

// system
#include <set>
#include <string>

// local
#include <XdgUtils/DesktopEntry/DesktopEntry.h>
#include <appimage/desktop_integration/IntegrationManager.h>

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
                 * Set the application actions that must be appended to the desktop entry on edit.
                 * @param additionalApplicationActions
                 */
                void setAdditionalApplicationActions(std::unordered_map<std::string, std::string> additionalApplicationActions);

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
                std::unordered_map<std::string, std::string> additionalApplicationActions;

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
                 * The new names will have the following structure "<oldApplicationName> (<appImageVersion>)"
                 *
                 * If the appImageVersion is not set the value from "Desktop Entry/X-AppImage-Version" will be used instead.
                 * If none of both options are valid the names will remain unchanged.
                 */
                void appendVersionToName(XdgUtils::DesktopEntry::DesktopEntry& entry);

                /**
                 * @brief Append the additionalApplicationActions to <entry>
                 * The desktop entry actions must follow the specification for additional application actions at https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s11.html.
                 * The map key should be the action identifier and the value the action fields in a plain string i.e.:
                 *
                 *  std::map<std::string, std::string> applicationActions = {{"Remove",
                 *                                                    "[Desktop Action Remove]\n"
                 *                                                    "Name=\"Remove application\"\n"
                 *                                                    "Icon=remove\n"
                 *                                                    "Exec=remove-appimage-helper /path/to/the/AppImage\n"}};
                 *
                 *
                 * @param entry
                 */
                void appendApplicationActions(XdgUtils::DesktopEntry::DesktopEntry& entry);
            };

        }
    }
}
