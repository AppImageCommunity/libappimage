function(configure_appimage_module target)
    # targets are called lib* already, so CMake shouldn't add another lib prefix to the actual files
    set_target_properties(${target}
        PROPERTIES PREFIX ""
        PUBLIC_HEADER ${libappimage_public_header}
        POSITION_INDEPENDENT_CODE ON
    )

    target_compile_definitions(${target}
        # Support Large Files
        PRIVATE -D_FILE_OFFSET_BITS=64
        PRIVATE -D_LARGEFILE_SOURCE

        PRIVATE -DGIT_COMMIT="${GIT_COMMIT}"
        PRIVATE -DENABLE_BINRELOC
        )

    target_include_directories(${target}
        PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
        PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/libappimage>
        INTERFACE $<INSTALL_INTERFACE:include/>
        )

    target_link_libraries(${target}
        PRIVATE libarchive
        PRIVATE xdg-basedir
        # not linking publicly to squashfuse as headers are not needed when using libappimage
        # unit tests etc., which use squashfuse directly, must link to it explicitly
        PRIVATE libsquashfuse
        PRIVATE xz
        PUBLIC libappimage_shared
        PUBLIC pthread
        PUBLIC libglib
        PUBLIC libgobject
        PUBLIC libgio
        PUBLIC libzlib
        PUBLIC libcairo
        )
endfunction()
