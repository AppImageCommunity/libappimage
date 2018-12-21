add_library(appimage_utils OBJECT MagicBytesChecker.cpp Elf.cpp Elf.h)

# Generic module configuration
configure_libappimage_module(appimage_utils)
