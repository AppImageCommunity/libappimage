add_library(
    desktop_integration_utils OBJECT
    XdgUserDirs.cpp
    Environment.cpp
    HashLib.cpp
)

target_include_directories(desktop_integration_utils PRIVATE $<TARGET_PROPERTY:libappimage_hashlib,INTERFACE_INCLUDE_DIRECTORIES>)
configure_libappimage_module(desktop_integration_utils)
