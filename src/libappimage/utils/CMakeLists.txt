add_library(appimage_utils OBJECT magic_bytes_checker.cpp)

# Generic module configuration
configure_libappimage_module(appimage_utils)

# Specific module configuration
target_include_directories(appimage_utils PUBLIC $<TARGET_PROPERTY:libglib,INTERFACE_INCLUDE_DIRECTORIES>)
