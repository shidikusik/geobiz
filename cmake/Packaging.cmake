# SPDX-License-Identifier: MIT
#
# Packaging.cmake — CPack configuration.
#
# Produces:
#   * Windows: an NSIS installer (.exe) and a portable ZIP.
#   * Linux:   a .tar.gz archive (the AppImage/Flatpak flows are driven from the
#              dedicated folders and CI, see flatpak/ and .github/workflows).
#
# The Windows runtime (Qt DLLs, QML, WebEngine) is expected to be staged next to
# the executable by `windeployqt` before `cpack` runs — see installer/windows.

set(CPACK_PACKAGE_NAME "GeoBiz Uzbekistan")
set(CPACK_PACKAGE_VENDOR "GeoBiz")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "GeoBiz Uzbekistan")
set(CPACK_PACKAGE_CONTACT "support@geobiz.uz")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "GeoBiz Uzbekistan")
    set(CPACK_NSIS_PACKAGE_NAME "GeoBiz Uzbekistan")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH OFF)
    # The .ico is generated from assets/icons/app.svg during the Windows CI job;
    # only reference it when present so a bare `cpack` still succeeds.
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/installer/windows/app.ico")
        set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/installer/windows/app.ico")
        set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/installer/windows/app.ico")
    endif()
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\GeoBizUzbekistan.exe")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\GeoBiz Uzbekistan.lnk' '$INSTDIR\\\\bin\\\\GeoBizUzbekistan.exe'")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\GeoBiz Uzbekistan.lnk'")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
