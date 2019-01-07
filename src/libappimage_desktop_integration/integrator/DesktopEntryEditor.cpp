// system
#include <string>
#include <sstream>
#include <algorithm>

// libraries
#include <XdgUtils/DesktopEntry/DesktopEntryExecValue.h>
#include <XdgUtils/DesktopEntry/DesktopEntryStringsValue.h>

// local
#include "DesktopEntryEditor.h"
#include "DesktopIntegrationErrors.h"


using namespace XdgUtils::DesktopEntry;

namespace appimage {
    namespace desktop_integration {
        namespace integrator {
            void DesktopEntryEditor::setAppImagePath(const std::string& appImagePath) {
                DesktopEntryEditor::appImagePath = appImagePath;
            }

            void DesktopEntryEditor::edit(XdgUtils::DesktopEntry::DesktopEntry& desktopEntry) {
                if (!desktopEntry.exists("Desktop Entry/Exec"))
                    throw DesktopEntryBuildError("Missing Desktop Entry");

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
                    appImageVersion = desktopEntry.get("Desktop Entry/X-AppImage-Version");

                    // find name entries
                    std::vector<std::string> nameEntriesPaths;
                    for (const auto& path: desktopEntry.paths())
                        if (path.find("Desktop Entry/Name") != std::string::npos)
                            nameEntriesPaths.emplace_back(path);

                    for (const auto& path: nameEntriesPaths) {
                        std::string name = desktopEntry.get(path);

                        // Skip version is already part of the name
                        if (name.find(appImageVersion) != std::string::npos)
                            continue;

                        std::stringstream newName;
                        newName << name << " (" << appImageVersion << ')';
                        desktopEntry.set(path, newName.str());

                        // Save old name value at <group>/X-AppImage-Old-Name<locale>
                        std::string groupPathSection;
                        const auto& groupSplitPos = path.find('/');
                        if (groupSplitPos != std::string::npos)
                            groupPathSection = path.substr(0, groupSplitPos);

                        std::string localePathSection;
                        const auto& localeStartPos = path.find('[');
                        if (localeStartPos != std::string::npos)
                            localePathSection = path.substr(path.find('['));

                        std::stringstream oldNameEntryPath;
                        oldNameEntryPath << groupPathSection << "/X-AppImage-Old-Name" << localePathSection;
                        desktopEntry.set(oldNameEntryPath.str(), name);
                    }
                }
            }

            void DesktopEntryEditor::setIcons(XdgUtils::DesktopEntry::DesktopEntry& desktopEntry) {
                if (identifier.empty())
                    throw DesktopEntryBuildError("Missing AppImage UUID");

                // retrieve all icon keys
                std::vector<std::string> iconEntriesPaths;
                for (const auto& path: desktopEntry.paths())
                    if (path.find("/Icon") != std::string::npos)
                        iconEntriesPaths.emplace_back(path);

                for (const auto& path: iconEntriesPaths) {
                    std::string icon = desktopEntry.get(path);

                    std::stringstream newIcon;
                    newIcon << vendorPrefix << "_" << identifier << "_" + icon;

                    desktopEntry.set(path, newIcon.str());

                    // Save old icon value at <group>/X-AppImage-Old-Icon<locale>
                    std::string groupPathSection;
                    const auto& groupSplitPos = path.find('/');
                    if (groupSplitPos != std::string::npos)
                        groupPathSection = path.substr(0, groupSplitPos);

                    std::string localePathSection;
                    const auto& localeStartPos = path.find('[');
                    if (localeStartPos != std::string::npos)
                        localePathSection = path.substr(path.find('['));

                    std::stringstream oldIconPath;
                    oldIconPath << groupPathSection << "/X-AppImage-Old-Icon" << localePathSection;
                    desktopEntry.set(oldIconPath.str(), icon);
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
