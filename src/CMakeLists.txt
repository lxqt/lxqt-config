project(lxqt-config)
add_subdirectory(qcategorizedview)

set(QTX_INCLUDE_DIRS  "")
set(QTX_LIBRARIES Qt5::Widgets Qt5::Xml)

include_directories (
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QTX_INCLUDE_DIRS}
    ${LXQT_INCLUDE_DIRS}
    ${QTXDG_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/qcategorizedview"
)

set(lxqt-config_HDRS "")

set(lxqt-config_SRCS
    main.cpp
    mainwindow.cpp
)

set(lxqt-config_MOCS
    mainwindow.h
)

set(lxqt-config_UIS
    mainwindow.ui
)

qt5_wrap_ui(UIS ${lxqt-config_UIS})

# Translations **********************************
lxqt_translate_ts(QM_FILES SOURCES
    UPDATE_TRANSLATIONS ${UPDATE_TRANSLATIONS}
    ${lxqt-config_HDRS}
    ${lxqt-config_SRCS}
    ${lxqt-config_UIS}
    INSTALL_DIR "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_app_translation_loader(QM_LOADER ${PROJECT_NAME})
lxqt_translate_desktop(DESKTOP_FILES SOURCES lxqt-config.desktop.in)

#************************************************

add_executable(lxqt-config ${lxqt-config_SRCS} ${UIS} ${MOCS} ${DESKTOP_FILES} ${QM_FILES} ${QM_LOADER})
target_link_libraries(lxqt-config
    ${QTX_LIBRARIES}
    ${QTXDG_LIBRARIES}
    ${LXQT_LIBRARIES}
)
# helper static lib
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/qcategorizedview)
target_link_libraries(lxqt-config qcategorizedview)

install(TARGETS lxqt-config RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")
install(FILES lxqt-config.menu DESTINATION /etc/xdg/menus/)
