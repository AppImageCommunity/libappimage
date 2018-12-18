add_library(appimage_desktop_integration
    integrator.h
    integrator.cpp
)

target_link_libraries(appimage_desktop_integration PUBLIC libappimage_0.2 )
