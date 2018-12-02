#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AppImageFormat.h"
#include "AppImageIterator.h"

namespace AppImage {

    class AppImage {
        std::string path;
        Format format;
    public:
        explicit AppImage(const std::string& path);

        virtual ~AppImage();

        const std::string& getPath() const;

        Format getFormat() const;

        static Format getFormat(const std::string& path);

        AppImageIterator files();
    };
}
