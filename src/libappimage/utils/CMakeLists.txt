set(
    APPIMAGE_UTILS_SRCS
    MagicBytesChecker.cpp
    ElfFile.cpp
    hashlib.cpp
    UrlEncoder.cpp
    IconHandle.cpp
    Logger.cpp
    path_utils.cpp
    resources_extractor/ResourcesExtractor.cpp
    resources_extractor/PayloadEntriesCache.cpp
    StringSanitizer.cpp
    StringSanitizer.h
)

set(APPIMAGE_UTILS_SRCS ${APPIMAGE_UTILS_SRCS} IconHandleCairoRsvg.cpp)
add_library(appimage_utils OBJECT ${APPIMAGE_UTILS_SRCS})

# Generic module configuration
configure_libappimage_module(appimage_utils)
target_include_directories(appimage_utils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(appimage_utils
    PRIVATE Boost::boost
    PRIVATE libappimage_hashlib
    PRIVATE XdgUtils::DesktopEntry
    PRIVATE XdgUtils::BaseDir
    PUBLIC libcairo
    PUBLIC librsvg
)
target_compile_definitions(appimage_utils PUBLIC ${IMAGE_MANIPULATION_BACKEND_COMPILE_DEFINITIONS})
