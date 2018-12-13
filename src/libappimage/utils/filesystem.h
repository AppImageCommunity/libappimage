#pragma once

#include <string>

namespace appimage {
    namespace utils {
        namespace filesystem {
            /**
             * Get the parent path of <path>
             * @param path
             * @return parent path if succeeds or empty string
             */
            std::string parentPath(const std::string& path);

            /**
             * Create a directory including its parents
             * @param path
             */
            void createDirectories(const std::string& path);

            /**
             * Remove file/dir and all its contents
             * @param path
             */
            void removeRecursive(const std::string& path);

            /**
             * Create a temporary dir at /tmp/<baseName>-XXXXXX, 'X' chars will be replaced by random characters.
             *
             * @param baseName
             * @return newly created dir path or an empty string
             */
            std::string createTempDir(const std::string& baseName);

            /**
             * Copy a file from <source> to <target>
             * @param source
             * @param target
             * @return true if all went ok, otherwise false
             */
            bool copyFile(const char* source, const char* target);
        }
    }
}
