add_subdirectory(impl)

set(appimage_core_srcs
    appimage.cpp
    traversal.h
    file_istream.cpp
    files_iterator.cpp
)

add_library(appimage_core OBJECT ${appimage_core_srcs} $<TARGET_OBJECTS:appimage_core_impl>)
configure_appimage_module(appimage_core)
