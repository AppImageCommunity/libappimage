#include <sstream>

#include "DesktopEntryBuilder.h"
#include "DesktopIntegrationErrors.h"

#include <XdgUtils/DesktopEntry/DesktopEntryExecValue.h>
#include <XdgUtils/DesktopEntry/DesktopEntryStringsValue.h>

using namespace XdgUtils::DesktopEntry;

namespace appimage {
    namespace desktop_integration {
        const std::string& DesktopEntryBuilder::getAppImagePath() const {
            return appImagePath;
        }

        void DesktopEntryBuilder::setAppImagePath(const std::string& appImagePath) {
            DesktopEntryBuilder::appImagePath = appImagePath;
        }

        void DesktopEntryBuilder::setBaseDesktopFile(std::istream& data) {
            data >> desktopEntry;
        }

        std::string DesktopEntryBuilder::build() {
            if (!desktopEntry.exists("Desktop Entry/Exec"))
                throw DesktopEntryBuildError("Missing Desktop Entry");


            setExecPaths();


            std::stringstream result;
            result << desktopEntry;
            return result.str();
        }

        void DesktopEntryBuilder::setExecPaths()  {
            // Edit "Desktop Entry/Exec"
            DesktopEntryExecValue execValue(desktopEntry.get("Desktop Entry/Exec"));
            execValue[0] = appImagePath;
            desktopEntry.set("Desktop Entry/Exec", execValue.dump());

            // Edit TryExec
            desktopEntry.set("Desktop Entry/TryExec", appImagePath);

            // modify actions Exec entry
            DesktopEntryStringsValue actions(desktopEntry.get("Desktop Entry/Actions"));
            for (int i = 0; i < actions.size(); i++) {
                std::__cxx11::string keyPath = "Desktop Action " + actions[i] + "/Exec";

                DesktopEntryExecValue actionExecValue(desktopEntry.get(keyPath));
                actionExecValue[0] = appImagePath;
                desktopEntry.set(keyPath, actionExecValue.dump());
            }
        }

        const std::string& DesktopEntryBuilder::getAppImageVersion() const {
            return appImageVersion;
        }

        void DesktopEntryBuilder::setAppImageVersion(const std::string& appImageVersion) {
            DesktopEntryBuilder::appImageVersion = appImageVersion;
        }
    }
}
