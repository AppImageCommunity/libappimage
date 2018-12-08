#pragma once

#include <string>

namespace appimage {
    namespace utils {
        namespace filesystem {
            std::string parentPath(const std::string& path);

            void createDirectories(const std::string& path);
        }
    }
}
