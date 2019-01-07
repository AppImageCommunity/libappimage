// system
#include <iostream>
#include <algorithm>

// local
#include "appimage/core/AppImage.h"
#include "utils/MagicBytesChecker.h"
#include "utils/Elf.h"

namespace appimage {
    namespace core {

        /**
         * Implementation of the opaque pointer patter for the appimage class
         * see https://en.wikipedia.org/wiki/Opaque_pointer
         */
        struct AppImage::appimage_priv {
            std::string path;
            FORMAT format = UNKNOWN;

            static FORMAT getFormat(const std::string& path);
        };

        AppImage::AppImage(const std::string& path) : d_ptr(new appimage_priv()) {
            d_ptr->path = path;
            d_ptr->format = d_ptr->getFormat(path);

            if (d_ptr->format == UNKNOWN)
                throw core::AppImageError("Unknown AppImage format");
        }

        const std::string& AppImage::getPath() const {
            return d_ptr->path;
        }

        FORMAT AppImage::getFormat() const {
            return d_ptr->format;
        }

        FORMAT AppImage::appimage_priv::getFormat(const std::string& path) {
            utils::magic_bytes_checker magicBytesChecker(path);
            if (magicBytesChecker.hasAppImageType1Signature())
                return TYPE_1;

            if (magicBytesChecker.hasAppImageType2Signature())
                return TYPE_2;

            if (magicBytesChecker.hasIso9660Signature() && magicBytesChecker.hasElfSignature()) {
                std::cerr << "WARNING: " << path << " seems to be a Type 1 AppImage without magic bytes."
                          << std::endl;
                return TYPE_1;
            }

            return UNKNOWN;
        }

        AppImage::~AppImage() {}

        FilesIterator AppImage::files() {
            return FilesIterator(d_ptr->path, d_ptr->format);
        }

        off_t AppImage::getPayloadOffset() const {
            utils::Elf elf(d_ptr->path);

            return elf.getSize();
        }

    }
}
