# build static helper class first
project(lxqt-config)
add_subdirectory(qcategorizedview)

if(USE_QT5)
  set(QTX_INCLUDE_DIRS  "")
  set(QTX_LIBRARIES Qt5::Widgets Qt5::Xml)
else()
  set(QTX_INCLUDE_DIRS  ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_HEADERS_DIR})
  set(QTX_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY})
endif()

include_directories (
        ${CMAKE_CURRENT_BINARY_DIR}
        ${QTX_INCLUDE_DIRS}
        ${LXQT_INCLUDE_DIRS}
        ${QTXDG_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/qcategorizedview
        )
add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")

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
# QT4_WRAP_CPP(MOCS ${lxqt-config_MOCS})

if(USE_QT5)
    qt5_wrap_ui(UIS ${lxqt-config_UIS})
else()
    qt4_wrap_ui(UIS ${lxqt-config_UIS})
endif()

# Translations **********************************
include(LxQtTranslate)
lxqt_translate_ts(QM_FILES
    SOURCES
        ${lxqt-config_HDRS}
        ${lxqt-config_SRCS}
        ${lxqt-config_UIS}
    INSTALLATION_DIR
        ${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}
)

lxqt_translate_desktop(DESKTOP_FILES
    SOURCES
        lxqt-config.desktop.in
)
#************************************************

add_executable ( lxqt-config ${lxqt-config_SRCS} ${UIS} ${MOCS} ${DESKTOP_FILES} ${QM_FILES})
target_link_libraries ( lxqt-config
                ${QTX_LIBRARIES}
                ${QTXDG_LIBRARIES}
                ${LXQT_LIBRARIES})
# helper static lib
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/qcategorizedview)
target_link_libraries( lxqt-config qcategorizedview)

install(TARGETS lxqt-config RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
install(FILES lxqt-config.menu DESTINATION /etc/xdg/menus/)
