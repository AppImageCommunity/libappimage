// system
#include <string>
#include <sstream>
#include <algorithm>

// libraries
#include <XdgUtils/DesktopEntry/DesktopEntryExecValue.h>
#include <XdgUtils/DesktopEntry/DesktopEntryStringsValue.h>

// local
#include "DesktopEntryBuilder.h"
#include "DesktopIntegrationErrors.h"


using namespace XdgUtils::DesktopEntry;

namespace appimage {
    namespace desktop_integration {

        void DesktopEntryBuilder::setAppImagePath(const std::string& appImagePath) {
            DesktopEntryBuilder::appImagePath = appImagePath;
        }

        void DesktopEntryBuilder::setBaseDesktopFile(std::istream& data) {
            data >> desktopEntry;
        }

        std::string DesktopEntryBuilder::build() {
            if (!desktopEntry.exists("Desktop Entry/Exec"))
                throw DesktopEntryBuildError("Missing Desktop Entry");

            // set default vendor prefix
            if (vendorPrefix.empty())
                vendorPrefix = "appimagekit";

            setExecPaths();


            setIcons();

            std::stringstream result;
            result << desktopEntry;
            return result.str();
        }

        void DesktopEntryBuilder::setIcons() {
            if (uuid.empty())
                throw DesktopEntryBuildError("Missing AppImage UUID");

            // retrieve all icon keys
            std::vector<std::string> iconEntriesPaths;
            for (const auto& path: desktopEntry.paths())
                if (path.find("/Icon") != std::string::npos)
                    iconEntriesPaths.emplace_back(path);

            for (const auto& path: iconEntriesPaths) {
                std::string icon = desktopEntry.get(path);
                desktopEntry.set(path, vendorPrefix + "_" + uuid + "_" + icon);

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

        void DesktopEntryBuilder::setExecPaths() {
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

        void DesktopEntryBuilder::setAppImageVersion(const std::string& appImageVersion) {
            DesktopEntryBuilder::appImageVersion = appImageVersion;
        }

        void DesktopEntryBuilder::setUuid(const std::__cxx11::basic_string<char>& uuid) {
            DesktopEntryBuilder::uuid = uuid;
        }

        void DesktopEntryBuilder::setVendorPrefix(const std::string& vendorPrefix) {
            DesktopEntryBuilder::vendorPrefix = vendorPrefix;
        }
    }
}
