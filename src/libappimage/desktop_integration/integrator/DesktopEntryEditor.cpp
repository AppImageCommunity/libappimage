// system
#include <string>
#include <sstream>
#include <algorithm>

// libraries
#include <XdgUtils/DesktopEntry/DesktopEntryExecValue.h>
#include <XdgUtils/DesktopEntry/DesktopEntryStringsValue.h>
#include <XdgUtils/DesktopEntry/DesktopEntryKeyPath.h>
#include <map>
#include <utility>

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

                appendApplicationActions(desktopEntry);

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
                    std::string icon = desktopEntry.get(path);

                    // create new icon name as "<vendorPrefix>_<uuid>_<oldIconName>"
                    std::stringstream newIcon;
                    newIcon << vendorPrefix << "_" << identifier << "_" + icon;

                    desktopEntry.set(path, newIcon.str());

                    // Save old icon value at <group>/X-AppImage-Old-Icon<locale>
                    DesktopEntryKeyPath oldValueKeyPath(path);
                    oldValueKeyPath.setKey("X-AppImage-Old-Icon");
                    desktopEntry.set(oldValueKeyPath.string(), icon);
                }
            }

            void DesktopEntryEditor::setAdditionalApplicationActions(std::unordered_map<std::string, std::string> additionalApplicationActions) {
                DesktopEntryEditor::additionalApplicationActions = std::move(additionalApplicationActions);
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

            void DesktopEntryEditor::appendApplicationActions(XdgUtils::DesktopEntry::DesktopEntry &entry) {
                for (auto itr = additionalApplicationActions.begin(); itr != additionalApplicationActions.end(); ++itr) {
                    try {
                        // validate correctness of the action specification
                        std::stringstream stringstream(itr->second);
                        XdgUtils::DesktopEntry::DesktopEntry action(stringstream);

                        // Add action
                        std::string actionsString = static_cast<std::string>(entry["Desktop Entry/Actions"]);
                        DesktopEntryStringsValue actions(actionsString);

                        actions.append(itr->first);
                        entry.set("Desktop Entry/Actions", actions.dump());

                        // Add action definition
                        for (const auto &path: action.paths())
                            entry[path] = action.get(path);

                    } catch (const DesktopEntryError &error) {
                        throw DesktopEntryEditError(std::string("Malformed action: ") + error.what());
                    }
                }
            }
        }
    }
}
