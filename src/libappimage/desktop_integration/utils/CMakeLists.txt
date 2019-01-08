add_library(
    appimage_desktop_integration_utils OBJECT
    HashLib.cpp
)

target_include_directories(appimage_desktop_integration_utils PRIVATE $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>)
configure_libappimage_module(appimage_desktop_integration_utils)
