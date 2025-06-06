// system
#include <string>
#include <sstream>
#include <algorithm>

// libraries
#include <XdgUtils/DesktopEntry/DesktopEntryExecValue.h>
#include <XdgUtils/DesktopEntry/DesktopEntryStringsValue.h>
#include <XdgUtils/DesktopEntry/DesktopEntryKeyPath.h>
#include <utils/StringSanitizer.h>

// local
#include "DesktopEntryEditor.h"
#include "DesktopEntryEditError.h"


using namespace XdgUtils::DesktopEntry;

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            void DesktopEntryEditor::setAppImagePath(const std::string& appImagePath) {
                DesktopEntryEditor::appImagePath = appImagePath;
            }

            void DesktopEntryEditor::edit(XdgUtils::DesktopEntry::DesktopEntry& desktopEntry) {
                if (!desktopEntry.exists("Desktop Entry/Exec"))
                    throw DesktopEntryEditError("Missing Desktop Entry");

                // set default vendor prefix
                if (vendorPrefix.empty())
                    vendorPrefix = "appimagekit";

                setExecPaths(desktopEntry);

                setIcons(desktopEntry);

                appendVersionToName(desktopEntry);

                // set identifier
                desktopEntry.set("Desktop Entry/X-AppImage-Identifier", identifier);
            }

            void DesktopEntryEditor::setAppImageVersion(const std::string& appImageVersion) {
                DesktopEntryEditor::appImageVersion = appImageVersion;
            }

            void DesktopEntryEditor::setIdentifier(const std::string& uuid) {
                DesktopEntryEditor::identifier = uuid;
            }

            void DesktopEntryEditor::setVendorPrefix(const std::string& vendorPrefix) {
                DesktopEntryEditor::vendorPrefix = vendorPrefix;
            }

            void DesktopEntryEditor::appendVersionToName(XdgUtils::DesktopEntry::DesktopEntry& desktopEntry) {
                // AppImage Version can be set from an external source like appstream.xml
                if (!appImageVersion.empty())
                    desktopEntry.set("Desktop Entry/X-AppImage-Version", appImageVersion);

                if (desktopEntry.exists("Desktop Entry/X-AppImage-Version")) {
                    // The AppImage Version can also be set by the author in the Desktop Entry
                    appImageVersion = desktopEntry.get("Desktop Entry/X-AppImage-Version");

                    // find name entries
                    std::vector<std::string> nameEntriesPaths;
                    for (const auto& path: desktopEntry.paths())
                        if (path.find("Desktop Entry/Name") != std::string::npos)
                            nameEntriesPaths.emplace_back(path);

                    for (const auto& path: nameEntriesPaths) {
                        std::string name = desktopEntry.get(path);

                        // Skip version if it's already part of the name
                        if (name.find(appImageVersion) != std::string::npos)
                            continue;

                        // create new name as "<oldApplicationName> (<appImageVersion>)"
                        std::stringstream newName;
                        newName << name << " (" << appImageVersion << ')';
                        desktopEntry.set(path, newName.str());

                        // Save old name value at <group>/X-AppImage-Old-Name<locale>
                        DesktopEntryKeyPath oldValueKeyPath(path);
                        oldValueKeyPath.setKey("X-AppImage-Old-Name");
                        desktopEntry.set(oldValueKeyPath.string(), name);
                    }
                }
            }

            void DesktopEntryEditor::setIcons(XdgUtils::DesktopEntry::DesktopEntry& desktopEntry) {
                if (identifier.empty())
                    throw DesktopEntryEditError("Missing AppImage UUID");

                // retrieve all icon key paths
                std::vector<std::string> iconEntriesPaths;
                for (const auto& path: desktopEntry.paths())
                    if (path.find("/Icon") != std::string::npos)
                        iconEntriesPaths.emplace_back(path);

                // add icon names
                for (const auto& path: iconEntriesPaths) {
                    std::string iconName = desktopEntry.get(path);

                    // Extract base name from icon name (e.g. "co.anysphere.cursor" -> "cursor")
                    size_t lastDot = iconName.find_last_of('.');
                    if (lastDot != std::string::npos) {
                        iconName = iconName.substr(lastDot + 1);
                    }

                    // create new icon name as "<vendorPrefix>_<uuid>_<oldIconName>"
                    std::stringstream newIcon;

                    // we don't trust the icon name inside the desktop file, so we sanitize the filename before
                    // calculating the integrated icon's path
                    // this keeps the filename understandable while mitigating risks for potential attacks
                    newIcon << vendorPrefix << "_" << identifier << "_" << StringSanitizer(iconName).sanitizeForPath();

                    desktopEntry.set(path, newIcon.str());

                    // Save old icon value at <group>/X-AppImage-Old-Icon<locale>
                    DesktopEntryKeyPath oldValueKeyPath(path);
                    oldValueKeyPath.setKey("X-AppImage-Old-Icon");
                    desktopEntry.set(oldValueKeyPath.string(), iconName);
                }
            }

            void DesktopEntryEditor::setExecPaths(XdgUtils::DesktopEntry::DesktopEntry& desktopEntry) {
                // Edit "Desktop Entry/Exec"
                DesktopEntryExecValue execValue(desktopEntry.get("Desktop Entry/Exec"));
                execValue[0] = appImagePath;
                desktopEntry.set("Desktop Entry/Exec", execValue.dump());

                // Edit TryExec
                desktopEntry.set("Desktop Entry/TryExec", appImagePath);

                // modify actions Exec entry
                DesktopEntryStringsValue actions(desktopEntry.get("Desktop Entry/Actions"));
                for (int i = 0; i < actions.size(); i++) {
                    std::string keyPath = "Desktop Action " + actions[i] + "/Exec";

                    DesktopEntryExecValue actionExecValue(desktopEntry.get(keyPath));
                    actionExecValue[0] = appImagePath;
                    desktopEntry.set(keyPath, actionExecValue.dump());
                }
            }
        }
    }
}
