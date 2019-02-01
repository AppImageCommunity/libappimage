#pragma once

// system
#include <memory>
#include <string>
#include <vector>

// local
#include <appimage/core/AppImageFormat.h>
#include <appimage/core/PayloadIterator.h>
#include <appimage/core/exceptions.h>

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
             * Creates an AppImage instance from <other> AppImage
             * @param other
             */
            AppImage(const AppImage& other);

            /**
             * Copy the <other> instance data into the current one.
             * @param other
             * @return
             */
            AppImage& operator=(const AppImage& other);

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
            AppImageFormat getFormat() const;

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
            PayloadIterator files() const;

            /**
             * Compare this to <rhs>
             * @param rhs
             * @return true if both are equal, false otherwise
             */
            bool operator==(const AppImage& rhs) const;

            /**
             * Compare this to <rhs>
             * @param rhs
             * @return true if they are different, false otherwise
             */
            bool operator!=(const AppImage& rhs) const;

        private:
            class Private;

            std::shared_ptr<Private> d;   // opaque pointer
        };
    }
}
