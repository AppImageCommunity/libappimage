#pragma once

#include <string>

namespace appimage::FileUtils {
    std::string parentPath(const std::string& path);

    void createDirectories(const std::string& path);
}
