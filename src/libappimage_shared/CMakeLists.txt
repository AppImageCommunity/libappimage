set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(libappimage_shared_public_header ${PROJECT_SOURCE_DIR}/include/appimage/appimage_shared.h)

add_library(libappimage_shared STATIC
    ${libappimage_shared_public_header}
    elf.c
    hexlify.c
    light_byteswap.h
    light_elf.h
    digest.c
)

target_compile_definitions(libappimage_shared
    # Support Large Files
    PUBLIC -D_FILE_OFFSET_BITS=64
)
set_target_properties(libappimage_shared PROPERTIES PREFIX "")
target_include_directories(libappimage_shared PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
set_property(TARGET libappimage_shared PROPERTY PUBLIC_HEADER ${libappimage_shared_public_header})
target_link_libraries(libappimage_shared PRIVATE libappimage_hashlib)

# install libappimage
install(TARGETS libappimage_shared
    EXPORT libappimageTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libappimage
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libappimage
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/appimage COMPONENT libappimage-dev
)
