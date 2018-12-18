// system
#include <iostream>
#include <algorithm>

// libraries
#include <libelf.h>
#include <gelf.h>
#include <fcntl.h>
#include <zconf.h>

// local
#include "core/appimage.h"
#include "core/exceptions.h"
#include "utils/magic_bytes_checker.h"

using namespace appimage;

/**
 * Implementation of the opaque pointer patter for the appimage class
 * see https://en.wikipedia.org/wiki/Opaque_pointer
 */
struct core::appimage::appimage_priv {
    std::string path;
    FORMAT format = UNKNOWN;
};

core::appimage::appimage(const std::string& path) : d_ptr(new appimage_priv()) {
    d_ptr->path = path;
    d_ptr->format = getFormat(path);

    if (d_ptr->format == UNKNOWN)
        throw core::AppImageError("Unknown AppImage format");
}

core::FORMAT core::appimage::getFormat(const std::string& path) {
    utils::magic_bytes_checker magicBytesChecker(path);
    if (magicBytesChecker.hasAppImageType1Signature())
        return TYPE_1;

    if (magicBytesChecker.hasAppImageType2Signature())
        return TYPE_2;

    if (magicBytesChecker.hasIso9660Signature() && magicBytesChecker.hasElfSignature()) {
        std::cerr << "WARNING: " << path << " seems to be a Type 1 AppImage without magic bytes." << std::endl;
        return TYPE_1;
    }

    return UNKNOWN;
}

const std::string& core::appimage::getPath() const {
    return d_ptr->path;
}

core::FORMAT core::appimage::getFormat() const {
    return d_ptr->format;
}

core::appimage::~appimage() {}

core::files_iterator core::appimage::files() {
    return files_iterator(d_ptr->path, d_ptr->format);
}

off_t appimage::core::appimage::getElfSize(const std::string& path) {
    // Initialize libelf
    if (elf_version(EV_CURRENT) == EV_NONE) // Assert that libelf was properly initialized
        throw AppImageError("Unable to initialize the ELF library");

    // Open file in read only mode
    off_t fd;
    if ((fd = open(path.c_str(), O_RDONLY, 0)) < 0)
        throw AppImageReadError("Failed to open " + path);

    Elf* e = elf_begin(fd, ELF_C_READ, nullptr);
    if (e == nullptr) {
        close(fd);      // release file
        throw AppImageReadError("elf_begin failed " + path);
    }

    off_t result;
    try {
        GElf_Ehdr ehdr;
        if ((gelf_getehdr(e, &ehdr)) == nullptr)
            throw AppImageReadError("gelf_getehdr failed " + path);

        // Get sections header table end
        size_t shnum;
        if ((elf_getshdrnum(e, &shnum)) < 0) // get the right value of shnum
            throw AppImageReadError("elf_getshdrnum failed " + path);

        off_t sht_end = ehdr.e_shoff + (ehdr.e_shentsize * shnum);

        // Get last section offset end
        Elf_Scn* scn = elf_getscn(e, shnum - 1);
        if (scn == nullptr)
            throw AppImageReadError("elf_getscn failed " + path);

        GElf_Shdr shdr;
        if ( (gelf_getshdr(scn, &shdr)) == nullptr)
            throw AppImageReadError("gelf_getshdr failed " + path);

        off_t last_section_end = shdr.sh_offset + shdr.sh_size;

        // select the largest value
        result = std::max(sht_end, last_section_end);
    } catch (const AppImageReadError&) {
        // libelf preserves the ownership of all the data returned from its functions so we must not release them
        elf_end(e);     // release Elf* resource
        close(fd);      // release file
        // rethrow
        throw;
    }

    // libelf preserves the ownership of all the data returned from its functions so we must not release them
    elf_end(e);     // release Elf* resource
    close(fd);      // release file

    return result;
}

off_t appimage::core::appimage::getElfSize() const {
    return getElfSize(d_ptr->path);
}
