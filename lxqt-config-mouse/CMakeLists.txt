project(lxqt-config-mouse)
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

set ( lxqt-config-mouse_HDRS
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

set ( lxqt-config-mouse_SRCS
    crtheme.cpp
    selectwnd.cpp
    xcr/xcrimg.cpp
    xcr/xcrxcur.cpp
    xcr/xcrthemefx.cpp
    xcr/xcrtheme.cpp
    xcr/xcrthemexp.cpp
    main.cpp
    cfgfile.cpp
    previewwidget.cpp
    itemdelegate.cpp
    thememodel.cpp
    warninglabel.cpp
	)

set(lxqt-config-mouse_UIS
     ui/selectwnd.ui
     ui/warninglabel.ui
)

find_package(ZLIB REQUIRED)
QT4_WRAP_UI(lxqt-config-mouse_CXX   ${lxqt-config-mouse_UIS})
QT4_WRAP_CPP(lxqt-config-mouse_MOCS ${lxqt-config-mouse_HDRS})
# Translations **********************************
include(LxQtTranslate)
lxqt_translate_ts(QM_FILES
    SOURCES
        ${lxqt-config-mouse_HDRS}
        ${lxqt-config-mouse_SRCS}
        ${lxqt-config-mouse_UIS}
    INSTALLATION_DIR
        ${CMAKE_INSTALL_PREFIX}/share/lxqt/lxqt-config
)

lxqt_translate_desktop(DESKTOP_FILES
    SOURCES
        lxqt-config-mouse.desktop.in
)
#************************************************

add_executable ( lxqt-config-mouse ${lxqt-config-mouse_SRCS} ${lxqt-config-mouse_CXX} ${lxqt-config-mouse_MOCS} ${DESKTOP_FILES} ${QM_FILES})
add_dependencies( lxqt-config-mouse lxqt qtxdg)
target_link_libraries ( lxqt-config-mouse  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY}
                                                 ${X11_X11_LIB} ${X11_Xcursor_LIB} lxqt qtxdg ${ZLIB_LIBRARY} ${X11_Xfixes_LIB})
# not needed probably ${X11_Xfixes_LIB})

INSTALL(TARGETS lxqt-config-mouse RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
