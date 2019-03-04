// system
extern "C" {
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
}

// local
#include "light_byteswap.h"
#include "ElfFile.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ELFDATANATIVE ELFDATA2LSB
#elif __BYTE_ORDER == __BIG_ENDIAN
#define ELFDATANATIVE ELFDATA2MSB
#else
#error "Unknown machine endian"
#endif

namespace appimage {
    namespace utils {
        ElfFile::ElfFile(const std::string& path) : path(path), fname(path.c_str()), ehdr({0x0}),
                                                    logger(Logger::instance()) {}

        uint16_t ElfFile::file16_to_cpu(uint16_t val) {
            if (ehdr.e_ident[EI_DATA] != ELFDATANATIVE)
                val = bswap_16(val);
            return val;
        }

        uint32_t ElfFile::file32_to_cpu(uint32_t val) {
            if (ehdr.e_ident[EI_DATA] != ELFDATANATIVE)
                val = bswap_32(val);
            return val;
        }

        uint64_t ElfFile::file64_to_cpu(uint64_t val) {
            if (ehdr.e_ident[EI_DATA] != ELFDATANATIVE)
                val = bswap_64(val);
            return val;
        }

        off_t ElfFile::read_elf32(FILE* fd) {
            Elf32_Ehdr ehdr32;
            Elf32_Shdr shdr32;
            off_t last_shdr_offset;
            ssize_t ret;
            off_t sht_end, last_section_end;

            fseeko(fd, 0, SEEK_SET);
            ret = fread(&ehdr32, 1, sizeof(ehdr32), fd);
            if (ret < 0 || (size_t) ret != sizeof(ehdr32)) {
                fprintf(stderr, "Read of ELF header from %s failed: %s\n",
                        fname, strerror(errno));
                return -1;
            }

            ehdr.e_shoff = file32_to_cpu(ehdr32.e_shoff);
            ehdr.e_shentsize = file16_to_cpu(ehdr32.e_shentsize);
            ehdr.e_shnum = file16_to_cpu(ehdr32.e_shnum);

            last_shdr_offset = ehdr.e_shoff + (ehdr.e_shentsize * (ehdr.e_shnum - 1));
            fseeko(fd, last_shdr_offset, SEEK_SET);
            ret = fread(&shdr32, 1, sizeof(shdr32), fd);
            if (ret < 0 || (size_t) ret != sizeof(shdr32)) {
                fprintf(stderr, "Read of ELF section header from %s failed: %s\n",
                        fname, strerror(errno));
                return -1;
            }

            /* ELF ends either with the table of section headers (SHT) or with a section. */
            sht_end = ehdr.e_shoff + (ehdr.e_shentsize * ehdr.e_shnum);
            last_section_end = file64_to_cpu(shdr32.sh_offset) + file64_to_cpu(shdr32.sh_size);
            return sht_end > last_section_end ? sht_end : last_section_end;
        }

        off_t ElfFile::read_elf64(FILE* fd) {
            Elf64_Ehdr ehdr64;
            Elf64_Shdr shdr64;
            off_t last_shdr_offset;
            off_t ret;
            off_t sht_end, last_section_end;

            fseeko(fd, 0, SEEK_SET);
            ret = fread(&ehdr64, 1, sizeof(ehdr64), fd);
            if (ret < 0 || (size_t) ret != sizeof(ehdr64)) {
                fprintf(stderr, "Read of ELF header from %s failed: %s\n",
                        fname, strerror(errno));
                return -1;
            }

            ehdr.e_shoff = file64_to_cpu(ehdr64.e_shoff);
            ehdr.e_shentsize = file16_to_cpu(ehdr64.e_shentsize);
            ehdr.e_shnum = file16_to_cpu(ehdr64.e_shnum);

            last_shdr_offset = ehdr.e_shoff + (ehdr.e_shentsize * (ehdr.e_shnum - 1));
            fseeko(fd, last_shdr_offset, SEEK_SET);
            ret = fread(&shdr64, 1, sizeof(shdr64), fd);
            if (ret < 0 || ret != sizeof(shdr64)) {
                logger->log(LogLevel::ERROR, std::string("Read of ELF section header from ") + fname
                                             + " failed: " + strerror(errno));
                return -1;
            }

            /* ELF ends either with the table of section headers (SHT) or with a section. */
            sht_end = ehdr.e_shoff + (ehdr.e_shentsize * ehdr.e_shnum);
            last_section_end = file64_to_cpu(shdr64.sh_offset) + file64_to_cpu(shdr64.sh_size);
            return sht_end > last_section_end ? sht_end : last_section_end;
        }

        ssize_t ElfFile::getSize() {
            off_t ret;
            FILE* fd = NULL;
            off_t size = -1;

            fd = fopen(fname, "rb");
            if (fd == NULL) {
                logger->log(LogLevel::ERROR, std::string("Cannot open ") + fname + ": " + strerror(errno));
                return -1;
            }
            ret = fread(ehdr.e_ident, 1, EI_NIDENT, fd);
            if (ret != EI_NIDENT) {
                logger->log(LogLevel::ERROR, std::string("Read of e_ident from ") + fname
                                             + " failed: " + strerror(errno));
                return -1;
            }
            if ((ehdr.e_ident[EI_DATA] != ELFDATA2LSB) &&
                (ehdr.e_ident[EI_DATA] != ELFDATA2MSB)) {
                logger->log(LogLevel::ERROR, "Unknown ELF data order " + std::to_string(ehdr.e_ident[EI_DATA]));
                return -1;
            }
            if (ehdr.e_ident[EI_CLASS] == ELFCLASS32) {
                size = read_elf32(fd);
            } else if (ehdr.e_ident[EI_CLASS] == ELFCLASS64) {
                size = read_elf64(fd);
            } else {
                logger->log(LogLevel::ERROR, "Unknown ELF class: " + std::to_string(ehdr.e_ident[EI_CLASS]));
                return -1;
            }

            fclose(fd);
            return size;
        }
    }
}
