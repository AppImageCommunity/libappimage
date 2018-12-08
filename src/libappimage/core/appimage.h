#pragma once

#include <memory>
#include <string>
#include <vector>

#include "format.h"
#include "files_iterator.h"

namespace appimage {
    namespace core {
        class appimage {
            std::string path;
            FORMAT format;
        public:
            explicit appimage(const std::string& path);

            virtual ~appimage();

            const std::string& getPath() const;

            FORMAT getFormat() const;

            static FORMAT getFormat(const std::string& path);

            files_iterator files();
        };
    }
}
