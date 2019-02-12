add_library(
    appimage_utils OBJECT
    MagicBytesChecker.cpp
    ElfFile.cpp
    HashLib.cpp
    UrlEncoder.cpp
    IconHandle.cpp
    Logger.cpp
    path_utils.cpp
)

# Generic module configuration
configure_libappimage_module(appimage_utils)
target_include_directories(
    appimage_utils
    PRIVATE $<TARGET_PROPERTY:Boost::filesystem,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>
)
add_dependencies(appimage_utils Boost::filesystem libappimage_hashlib)
