add_library(appimage_desktop_integration
    integrator.h
    integrator.cpp
    $<TARGET_OBJECTS:desktop_integration_utils>
)

target_link_libraries(appimage_desktop_integration PUBLIC libappimage_0.2 )
