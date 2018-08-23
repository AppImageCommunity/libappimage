set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(libappimage_shared STATIC elf.c hexlify.c light_byteswap.h light_elf.h)
set_target_properties(libappimage_shared PROPERTIES PREFIX "")
target_include_directories(libappimage_shared PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# install libappimage
install(TARGETS libappimage_shared
    EXPORT libappimageTargets
    LIBRARY DESTINATION lib COMPONENT libappimage
    ARCHIVE DESTINATION lib/static COMPONENT libappimage
    PUBLIC_HEADER DESTINATION include/appimage COMPONENT libappimage-dev
)
