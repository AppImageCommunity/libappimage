message(STATUS "Generating test data AppImage")

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
    execute_process(
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/appimagetool-x86_64.AppImage ${TARGET_APPDIR}
        ${CMAKE_CURRENT_BINARY_DIR}/${APPIMAGE_NAME}.AppImage)
endif()
