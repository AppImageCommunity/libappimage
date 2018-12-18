// system
#include <iostream>

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
    if (elf_version(EV_CURRENT) == EV_NONE)
        throw AppImageError("Unable to initialize the ELF library");

    off_t fd;
    if ((fd = open(path.c_str(), O_RDONLY, 0)) < 0)
        throw AppImageReadError("Failed to open " + path);

    Elf* e = nullptr;
    if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
        throw AppImageReadError("elf_begin failed " + path);

    GElf_Ehdr ehdr;
    if ((gelf_getehdr(e, &ehdr)) == NULL)
        throw AppImageReadError("gelf_getehdr failed " + path);

    // Get sections header table end
    size_t shnum;
    elf_getshdrnum(e, &shnum); // get the right value of shnum
    off_t sht_end = ehdr.e_shoff + (ehdr.e_shentsize * shnum);

    // Get last section offset end
    Elf_Scn* scn = elf_getscn(e, shnum-1);
    GElf_Shdr shdr;
    gelf_getshdr(scn, &shdr);
    size_t last_section_end = shdr.sh_offset + shdr.sh_size;

    elf_end(e);
    close(fd);

    // return the largest value
    return sht_end > last_section_end ? sht_end : last_section_end;
}

off_t appimage::core::appimage::getElfSize() {
    return getElfSize(d_ptr->path);
}
