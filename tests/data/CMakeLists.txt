message(STATUS "Generating test data AppImage")

# Get appimagetool
set(APPIMAGETOOL_BIN ${CMAKE_CURRENT_BINARY_DIR}/appimagetool)
if(NOT EXISTS ${APPIMAGETOOL_BIN})
    if(DEFINED ENV{ARCH} )
        set(APPIMAGETOOL_BIN_ARCH $ENV{ARCH})
    else()
        set(APPIMAGETOOL_BIN_ARCH ${CMAKE_SYSTEM_PROCESSOR})
    endif()

    message(STATUS "Downloading appimagetool for ${APPIMAGETOOL_BIN_ARCH}")
    file(DOWNLOAD "https://github.com/AppImage/AppImageKit/releases/download/12/appimagetool-${APPIMAGETOOL_BIN_ARCH}.AppImage" "${APPIMAGETOOL_BIN}")

    execute_process(COMMAND chmod +x ${APPIMAGETOOL_BIN})
endif()
message(STATUS "Using appimagetool: ${APPIMAGETOOL_BIN}")

# Generate Echo AppImage with custom mime type package
set(APPIMAGE_NAME "echo.with.mimetype")
set(TARGET_APPDIR ${CMAKE_CURRENT_BINARY_DIR}/${APPIMAGE_NAME}.AppDir)

if(NOT EXISTS ${TARGET_APPDIR})
    message(STATUS "Generating ${APPIMAGE_NAME} AppDir")

    file(MAKE_DIRECTORY ${TARGET_APPDIR})
    configure_file(squashfs-root/echo.desktop.in ${TARGET_APPDIR}/echo.desktop @ONLY)
    file(COPY squashfs-root/.DirIcon DESTINATION ${TARGET_APPDIR})
    file(COPY squashfs-root/AppRun DESTINATION ${TARGET_APPDIR})
    file(COPY squashfs-root/utilities-terminal.svg DESTINATION ${TARGET_APPDIR})
    file(COPY squashfs-root/usr DESTINATION ${TARGET_APPDIR})

    message(STATUS "Creating ${APPIMAGE_NAME} AppImage")
    execute_process(COMMAND ${APPIMAGETOOL_BIN} ${TARGET_APPDIR} ${CMAKE_CURRENT_BINARY_DIR}/${APPIMAGE_NAME}.AppImage)
endif()
