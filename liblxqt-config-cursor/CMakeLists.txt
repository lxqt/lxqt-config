project(lxqt-config-cursor)
find_package(X11 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB REQUIRED xcb)
include_directories(${XCB_INCLUDE_DIRS})
link_libraries(${XCB_LIBRARIES})

include_directories (
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${X11_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/xcr
)

set(lxqt-config-cursor_HDRS
    thememodel.h
    previewwidget.h
    itemdelegate.h
    selectwnd.h
    warninglabel.h
)

set(lxqt-config-cursor_SRCS
    crtheme.cpp
    selectwnd.cpp
    xcr/xcrimg.cpp
    xcr/xcrxcur.cpp
    xcr/xcrthemefx.cpp
    xcr/xcrtheme.cpp
    xcr/xcrthemexp.cpp
    cfgfile.cpp
    previewwidget.cpp
    itemdelegate.cpp
    thememodel.cpp
    warninglabel.cpp
)

set(lxqt-config-cursor_UIS
    ui/selectwnd.ui
    ui/warninglabel.ui
)

find_package(ZLIB REQUIRED)

qt5_wrap_ui(lxqt-config-cursor_CXX   ${lxqt-config-cursor_UIS})

# Translations **********************************
lxqt_translate_ts(QM_FILES SOURCES
    UPDATE_TRANSLATIONS ${UPDATE_TRANSLATIONS}
    ${lxqt-config-cursor_HDRS}
    ${lxqt-config-cursor_SRCS}
    ${lxqt-config-cursor_UIS}
    INSTALL_DIR "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_app_translation_loader(lxqt-config-cursor_QM_LOADER ${PROJECT_NAME})

#************************************************

add_library(lxqt-config-cursor SHARED
    ${lxqt-config-cursor_SRCS} ${lxqt-config-cursor_CXX} ${lxqt-config-cursor_MOCS} ${DESKTOP_FILES} ${QM_FILES} ${lxqt-config-cursor_QM_LOADER})
target_link_libraries(lxqt-config-cursor
    Qt5::X11Extras
    Qt5::DBus
    Qt5::Xml
    ${X11_X11_LIB}
    ${X11_Xcursor_LIB}
    ${LXQT_LIBRARIES}
    ${QTXDG_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${X11_Xfixes_LIB}
)
# not needed probably ${X11_Xfixes_LIB})

install(TARGETS lxqt-config-cursor DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${DESKTOP_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")
