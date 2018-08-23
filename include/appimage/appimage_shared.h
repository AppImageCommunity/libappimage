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

#ifdef __cplusplus
}
#endif
