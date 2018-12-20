#pragma once
// system
#include <memory>
#include <string>
#include <vector>

// local
#include "Format.h"
#include "FilesIterator.h"

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


            /*
             * Calculate the size of an ELF file on disk based on the information in its header
             *
             * Example:
             *
             * ls -l   126584
             *
             * Calculation using the values also reported by readelf -h:
             * Start of section headers	e_shoff		124728
             * Size of section headers		e_shentsize	64
             * Number of section headers	e_shnum		29
             *
             * e_shoff + ( e_shentsize * e_shnum ) =	126584
             */
            off_t getElfSize() const;


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
