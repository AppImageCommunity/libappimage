// system
#include <iostream>
#include <algorithm>
#include <appimage/core/AppImage.h>


// local
#include "appimage/core/AppImage.h"
#include "utils/MagicBytesChecker.h"
#include "utils/ElfFile.h"

namespace appimage {
    namespace core {

        /**
         * Implementation of the opaque pointer patter for the appimage class
         * see https://en.wikipedia.org/wiki/Opaque_pointer
         */
        class AppImage::Private {
        public:
            std::string path;
            FORMAT format = INVALID;

            explicit Private(const std::string& path);

            static FORMAT getFormat(const std::string& path);

        };

        AppImage::AppImage(const std::string& path) : d(new Private(path)) {
        }

        const std::string& AppImage::getPath() const {
            return d->path;
        }

        FORMAT AppImage::getFormat() const {
            return d->format;
        }

        AppImage::Private::Private(const std::string& path) : path(path) {
            format = getFormat(path);

            if (format == INVALID)
                throw core::AppImageError("Unknown AppImage format");
        }

        FORMAT AppImage::Private::getFormat(const std::string& path) {
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

            return INVALID;
        }

        AppImage::~AppImage() = default;

        FilesIterator AppImage::files() {
            return FilesIterator(d->path, d->format);
        }

        off_t AppImage::getPayloadOffset() const {
            utils::ElfFile elf(d->path);

            return elf.getSize();
        }

        AppImage& AppImage::operator=(const AppImage& other) = default;

        AppImage::AppImage(const AppImage& other) = default;
    }
}
