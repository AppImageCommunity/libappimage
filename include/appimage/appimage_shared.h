#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdlib.h>

/*
 * Return the offset, and the length of an ELF section with a given name in a given ELF file
 */
bool appimage_get_elf_section_offset_and_length(const char* fname, const char* section_name, unsigned long* offset, unsigned long* length);

int appimage_print_hex(char* fname, unsigned long offset, unsigned long length);
int appimage_print_binary(char* fname, unsigned long offset, unsigned long length);

/*
 * Creates hexadecimal representation of a byte array. Allocates a new char array (string) with the correct size that
 * needs to be free()d.
 */
char* appimage_hexlify(const char* bytes, size_t numBytes);

/*
 * Calculate MD5 digest of AppImage file, skipping the signature and digest sections.
 *
 * The digest section must be skipped as the value calculated by this method is going to be embedded in it by default.
 *
 * The signature section must be skipped as the signature will not be available at the time this hash is calculated.
 *
 * The hash is _not_ compatible with tools like md5sum.
 *
 * You need to allocate a char array of at least 16 bytes (128 bit) and pass a reference to it as digest parameter.
 * The function will set it to the raw digest, without any kind of termination. Please use appimage_hexlify() if you
 * need a textual representation.
 *
 * Please beware that this calculation is only available for type 2 AppImages.
 */
bool appimage_type2_digest_md5(const char* fname, char* digest);

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
ssize_t appimage_get_elf_size(const char* fname);

#ifdef __cplusplus
}
#endif
