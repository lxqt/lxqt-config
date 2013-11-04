# build static helper class first
project(lxqt-config)
add_subdirectory(qcategorizedview)

include_directories (
        ${CMAKE_CURRENT_BINARY_DIR}
        ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
        ${LXQT_INCLUDE_DIRS}
        ${QTXDG_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/qcategorizedview
        )

set ( lxqt-config_HDRS
	)

set ( lxqt-config_SRCS
	main.cpp
	mainwindow.cpp
	)

set ( lxqt-config_MOCS
    mainwindow.h
	)

set( lxqt-config_UIS
    mainwindow.ui
)
QT4_WRAP_CPP(MOCS ${lxqt-config_MOCS})
QT4_WRAP_UI(UIS ${lxqt-config_UIS})
# Translations **********************************
include(LxQtTranslate)
lxqt_translate_ts(QM_FILES
    SOURCES
        ${lxqt-config_HDRS}
        ${lxqt-config_SRCS}
        ${lxqt-config_UIS}
)

lxqt_translate_desktop(DESKTOP_FILES
    SOURCES
        lxqt-config.desktop.in
)
#************************************************

add_executable ( lxqt-config ${lxqt-config_SRCS} ${UIS} ${MOCS} ${DESKTOP_FILES} ${QM_FILES})
add_dependencies( lxqt-config lxqt qtxdg)
target_link_libraries ( lxqt-config
                ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY}
                ${QTXDG_LIBRARIES}
                ${LXQT_LIBRARIES})
# helper static lib
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/qcategorizedview)
target_link_libraries( lxqt-config qcategorizedview)

INSTALL(TARGETS lxqt-config RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
install(FILES lxqt-config.menu DESTINATION /etc/xdg/menus/)
