project(lxqt-config-cursor)
find_package(X11 REQUIRED)

if(USE_QT5)
  set(QTX_INCLUDE_DIRS  "")
  set(QTX_LIBRARIES Qt5::Widgets Qt5::X11Extras Qt5::DBus Qt5::Xml)
  # We need XCB for Qt 5
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(XCB REQUIRED xcb)
  include_directories(${XCB_INCLUDE_DIRS})
  link_libraries(${XCB_LIBRARIES})
else()
  set(QTX_INCLUDE_DIRS  ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTDBUS_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR})
  set(QTX_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
endif()

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${QTX_INCLUDE_DIRS}
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

if(USE_QT5)
    qt5_wrap_ui(lxqt-config-cursor_CXX   ${lxqt-config-cursor_UIS})
else()
    qt4_wrap_ui(lxqt-config-cursor_CXX   ${lxqt-config-cursor_UIS})
    # qt4_wrap_cpp(lxqt-config-cursor_MOCS ${lxqt-config-cursor_HDRS})
endif()

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
target_link_libraries ( lxqt-config-cursor  ${QTX_LIBRARIES} ${X11_X11_LIB} ${X11_Xcursor_LIB} ${LXQT_LIBRARIES} ${QTXDG_LIBRARIES} ${ZLIB_LIBRARY} ${X11_Xfixes_LIB})
# not needed probably ${X11_Xfixes_LIB})

install(TARGETS lxqt-config-cursor DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
