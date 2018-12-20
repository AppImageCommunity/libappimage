add_subdirectory(utils)

add_library(
    appimage_desktop_integration SHARED
    Integrator.cpp
    $<TARGET_OBJECTS:desktop_integration_utils>
)

target_link_libraries(appimage_desktop_integration
    PUBLIC libappimage_0.2
    PRIVATE libappimage_hashlib
    PRIVATE linuxdeploy_desktopfile
    )

configure_libappimage_module(appimage_desktop_integration)
