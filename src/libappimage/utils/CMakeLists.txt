add_library(appimage_utils OBJECT MagicBytesChecker.cpp Elf.cpp Elf.h HashLib.cpp)

# Generic module configuration
configure_libappimage_module(appimage_utils)
target_include_directories( appimage_utils PUBLIC $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>)
