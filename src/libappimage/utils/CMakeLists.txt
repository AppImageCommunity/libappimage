add_library(
    appimage_utils OBJECT
    MagicBytesChecker.cpp
    ElfFile.cpp
    hashlib.cpp
    UrlEncoder.cpp
    IconHandle.cpp
    Logger.cpp
    path_utils.cpp
    resources_extractor/ResourcesExtractor.cpp
    resources_extractor/PayloadEntriesCache.cpp
)

# Generic module configuration
configure_libappimage_module(appimage_utils)
target_include_directories(
    appimage_utils
    PRIVATE $<TARGET_PROPERTY:Boost::filesystem,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:XdgUtils::DesktopEntry,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:XdgUtils::BaseDir,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>
)
add_dependencies(appimage_utils Boost::filesystem libappimage_hashlib XdgUtils::DesktopEntry XdgUtils::BaseDir)
