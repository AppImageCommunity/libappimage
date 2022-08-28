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

target_include_directories(core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(core
    PRIVATE Boost::boost
    PRIVATE libappimage_hashlib
    PRIVATE XdgUtils::DesktopEntry
    PRIVATE libarchive
    PRIVATE libsquashfuse
)

configure_libappimage_module(core)
