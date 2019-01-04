set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(public_header ${CMAKE_CURRENT_SOURCE_DIR}/include/hashlib.h)

add_library(libappimage_hashlib STATIC md5.c ${public_header})
set_target_properties(libappimage_hashlib PROPERTIES PREFIX "")
target_include_directories(libappimage_hashlib
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
)

# install libappimage
install(TARGETS libappimage_hashlib
    EXPORT libappimageTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libappimage
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libappimage
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/appimage COMPONENT libappimage-dev
)
