#pragma once

#include <memory>
#include <string>
#include <vector>

#include "format.h"
#include "files_iterator.h"

namespace appimage {
    namespace core {
        /**
         * An object of class <appimage> represents an existent AppImage file. Provides readonly methods to
         * access the AppImage information and contained files.
         */
        class appimage {
            std::string path;
            FORMAT format;
        public:
            /**
             * Open the AppImage at <path>.
             * @param path
             * @throw AppImageReadError if something goes wrong
             */
            explicit appimage(const std::string& path);

            virtual ~appimage();

            const std::string& getPath() const;

            FORMAT getFormat() const;

            static FORMAT getFormat(const std::string& path);

            files_iterator files();
        };
    }
}
