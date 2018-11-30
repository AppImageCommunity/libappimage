#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AppImageFormat.h"

namespace AppImage {
    class AppImage {
        std::string path;
        Format format;
    public:
        explicit AppImage(const std::string& path);

        const std::string& getPath() const;

        Format getFormat() const;

        static Format getFormat(const std::string& path);

    };
}
