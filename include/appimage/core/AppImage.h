#pragma once
// system
#include <memory>
#include <string>
#include <vector>

// local
#include <appimage/core/Format.h>
#include <appimage/core/FilesIterator.h>
#include <appimage/core/Exceptions.h>

namespace appimage {
    namespace core {
        /**
         * An object of class <appimage> represents an existent AppImage file. Provides readonly methods to
         * access the AppImage information and contained files.
         */
        class AppImage {
        public:
            /**
             * Open the AppImage at <path>.
             * @param path
             * @throw AppImageError if something goes wrong
             */
            explicit AppImage(const std::string& path);

            /**
             * Default destructor.
             *
             * Required by `std::shared_ptr` to work properly.
             */
            virtual ~AppImage();

            /**
             * @return AppImage file path
             */
            const std::string& getPath() const;

            /**
             * Inspect the magic bytes of the file to guess the AppImage <FORMAT>
             * @return AppImage <FORMAT>
             */
            FORMAT getFormat() const;

            /**
             * Calculate the offset in the AppImage file where is located the payload file system.
             *
             * @return offset where the payload filesystem is located.
             */
            off_t getPayloadOffset() const;

            /**
             * Provides a one way iterator to traverse and access the files contained inside the AppImage.
             * @return a files_iterator instance
             * @throw AppImageError if something goes wrong
             */
            FilesIterator files();

        private:
            struct appimage_priv;
            std::shared_ptr<appimage_priv> d_ptr;   // opaque pointer
        };
    }
}
