#pragma once

#include <XdgUtils/DesktopEntry/DesktopEntry.h>

namespace appimage {
    namespace desktop_integration {
        /**
         * @brief Edit a Desktop Entry from an AppImage to deploy it into the system.
         *
         * Taking a <baseDesktopEntry> as input this class allows to reset the 'Exec', and 'Icon' entries to new values.
         */
        class DesktopEntryBuilder {
            std::string appImagePath;
            std::string appImageVersion;
            XdgUtils::DesktopEntry::DesktopEntry desktopEntry;

        public:

            const std::string& getAppImagePath() const;

            void setAppImagePath(const std::string& appImagePath);

            const std::string& getAppImageVersion() const;

            void setAppImageVersion(const std::string& appImageVersion);

            void setBaseDesktopFile(std::istream& data);

            std::string build();

        private:

            /**
             * Set Exec and TryExec entries in the 'Desktop Entry' and 'Desktop Action' groups pointing to the
             * <appImagePath>.
             */
            void setExecPaths();
        };

    }
}
