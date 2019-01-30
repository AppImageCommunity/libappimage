add_library(
    core OBJECT

    AppImage.cpp
    Traversal.h
    Traversal.cpp
    PayloadIterator.cpp
    impl/TraversalType1.cpp
    impl/TraversalType2.cpp
    impl/StreambufType1.cpp
    impl/StreambufType2.cpp
)

target_include_directories(
    core

    PRIVATE $<TARGET_PROPERTY:Boost,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:XdgUtils,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libarchive,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:libsquashfuse,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

configure_libappimage_module(core)
add_dependencies(core Boost libappimage_hashlib XdgUtils libarchive libsquashfuse)
