project(lxqt-config-cursor)
find_package(X11 REQUIRED)

include_directories(${X11_INCLUDE_DIR})

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTDBUS_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR}
    ${X11_INCLUDE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/xcr
    )

set ( lxqt-config-cursor_HDRS
#    cfgfile.h
    thememodel.h
    previewwidget.h
#    xcr/xcrtheme.h
#    xcr/xcrimg.h
#    xcr/xcrthemexp.h
#    xcr/xcrthemefx.h
#    xcr/xcrxcur.h
    itemdelegate.h
#    main.h
#    crtheme.h
    selectwnd.h
    warninglabel.h
	)

set ( lxqt-config-cursor_SRCS
    crtheme.cpp
    selectwnd.cpp
    xcr/xcrimg.cpp
    xcr/xcrxcur.cpp
    xcr/xcrthemefx.cpp
    xcr/xcrtheme.cpp
    xcr/xcrthemexp.cpp
    # main.cpp
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
QT4_WRAP_UI(lxqt-config-cursor_CXX   ${lxqt-config-cursor_UIS})
QT4_WRAP_CPP(lxqt-config-cursor_MOCS ${lxqt-config-cursor_HDRS})
# Translations **********************************
include(LxQtTranslate)
lxqt_translate_ts(QM_FILES
    SOURCES
        ${lxqt-config-cursor_HDRS}
        ${lxqt-config-cursor_SRCS}
        ${lxqt-config-cursor_UIS}
    INSTALLATION_DIR
        ${CMAKE_INSTALL_PREFIX}/share/lxqt/lxqt-config
)

#************************************************

add_library ( lxqt-config-cursor SHARED ${lxqt-config-cursor_SRCS} ${lxqt-config-cursor_CXX} ${lxqt-config-cursor_MOCS} ${DESKTOP_FILES} ${QM_FILES})
add_dependencies( lxqt-config-cursor lxqt qtxdg)
target_link_libraries ( lxqt-config-cursor  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY}
                                                 ${X11_X11_LIB} ${X11_Xcursor_LIB} lxqt qtxdg ${ZLIB_LIBRARY} ${X11_Xfixes_LIB})
# not needed probably ${X11_Xfixes_LIB})

INSTALL(TARGETS lxqt-config-cursor DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
