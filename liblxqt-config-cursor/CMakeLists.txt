find_package(X11 REQUIRED)
find_package(XCB REQUIRED COMPONENTS XCB)
find_package(ZLIB REQUIRED)

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
    selectwnd.ui
    warninglabel.ui
)

# Translations **********************************
lxqt_translate_ts(QM_FILES
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${lxqt-config-cursor_HDRS}
        ${lxqt-config-cursor_SRCS}
        ${lxqt-config-cursor_UIS}
    TEMPLATE
         lxqt-config-cursor
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/lxqt-config-cursor"
)

lxqt_app_translation_loader(lxqt-config-cursor_QM_LOADER lxqt-config-cursor)

#************************************************

add_library(lxqt-config-cursor
    SHARED
        ${lxqt-config-cursor_SRCS}
        ${DESKTOP_FILES}
        ${QM_FILES}
        ${lxqt-config-cursor_QM_LOADER}
)

target_include_directories(lxqt-config-cursor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/xcr
)

target_link_libraries(lxqt-config-cursor
    Qt6::DBus
    Qt6::Xml
    X11::X11
    X11::Xcursor
    X11::Xfixes
    ZLIB::ZLIB
    XCB::XCB
    lxqt
)
# not needed probably ${X11_Xfixes_LIB})

install(TARGETS
    lxqt-config-cursor
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/${LXQT_CONFIG_PROJECT}"
    COMPONENT Runtime
)
install(FILES
    ${DESKTOP_FILES}
    DESTINATION "${CMAKE_INSTALL_DATAROOTDOR}/applications"
    COMPONENT Runtime
)

install(FILES
    man/lxqt-config-input.1
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
    COMPONENT Runtime
)
