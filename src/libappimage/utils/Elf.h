#pragma once

// system
#include <string>
#include <cstring>

// local
#include "light_elf.h"

namespace appimage {
    namespace utils {
        /**
         * Utility class to read elf files. Not meant to be feature complete
         */
        class Elf {
            std::string path;
            const char* fname;
            Elf64_Ehdr ehdr;
        public:
            explicit Elf(const std::string& path);


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
            ssize_t getSize();

        private:
            uint16_t file16_to_cpu(uint16_t val);

            uint32_t file32_to_cpu(uint32_t val);

            uint64_t file64_to_cpu(uint64_t val);

            off_t read_elf32(FILE* fd);

            off_t read_elf64(FILE* fd);

            /* Return the offset, and the length of an ELF section with a given name in a given ELF file */
            bool appimage_get_elf_section_offset_and_length(const char* section_name, unsigned long* offset,
                                                            unsigned long* length);

            char* read_file_offset_length(unsigned long offset, unsigned long length);
        };
    }
}
