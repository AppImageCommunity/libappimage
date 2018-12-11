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
        }
    }
}
