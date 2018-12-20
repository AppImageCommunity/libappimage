add_library(appimage_desktop_integration
    Integrator.h
    Integrator.cpp
    $<TARGET_OBJECTS:desktop_integration_utils>)

target_link_libraries(appimage_desktop_integration
    PUBLIC libappimage_0.2
    PRIVATE libappimage_hashlib
    PRIVATE linuxdeploy_desktopfile
)

target_include_directories(appimage_desktop_integration PRIVATE ..)
