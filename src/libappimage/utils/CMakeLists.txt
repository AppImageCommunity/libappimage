add_library(
    appimage_utils OBJECT
    MagicBytesChecker.cpp
    Elf.cpp
    HashLib.cpp
    UrlEncoder.cpp
    IconHandle.cpp
)

# Generic module configuration
configure_libappimage_module(appimage_utils)
target_include_directories(
    appimage_utils
    PRIVATE $<TARGET_PROPERTY:Boost,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>
)
add_dependencies(appimage_utils Boost libappimage_hashlib)