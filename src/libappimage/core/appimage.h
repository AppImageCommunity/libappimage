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
        public:
            /**
             * Open the AppImage at <path>.
             * @param path
             * @throw AppImageError if something goes wrong
             */
            explicit appimage(const std::string& path);

            virtual ~appimage();

            /**
             * @return AppImage file path
             */
            const std::string& getPath() const;

            /**
             * See <static FORMAT getFormat(path)>
             * @return AppImage format
             */
            FORMAT getFormat() const;

            /**
             * Inspect the magic bytes of the file pointed by <path> to guess the AppImage <FORMAT>
             * @param path
             * @return AppImage <FORMAT>
             */
            static FORMAT getFormat(const std::string& path);

            /**
             * Provides a one way iterator to traverse and access the files contained inside the AppImage.
             * @return a files_iterator instance
             * @throw AppImageError if something goes wrong
             */
            files_iterator files();

        private:
            struct appimage_priv;
            std::unique_ptr<appimage_priv> d_ptr;   // opaque pointer
        };
    }
}
