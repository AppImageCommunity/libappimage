#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AppImageFormat.h"
#include "AppImageIterator.h"

namespace appimage {

    class appimage {
        std::string path;
        Format format;
    public:
        explicit appimage(const std::string& path);

        virtual ~appimage();

        const std::string& getPath() const;

        Format getFormat() const;

        static Format getFormat(const std::string& path);

        AppImageIterator files();
    };
}
