add_library(
    legacy OBJECT

    libappimage.c
    appimage_handler.c
    type1.c
    type2.c
)

target_include_directories(
    legacy
    PUBLIC $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:Boost,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:XdgUtils,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libarchive,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libsquashfuse,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libglib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:xdg-basedir,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libcairo,INTERFACE_INCLUDE_DIRECTORIES>
)

configure_libappimage_module(legacy)
add_dependencies(legacy libappimage_hashlib libarchive libsquashfuse libglib xdg-basedir libcairo)
