project(lxqt-config-monitor)
set(QTX_INCLUDE_DIRS "")
set(QTX_LIBRARIES Qt5::Widgets Qt5::X11Extras)

add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")

include_directories (
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${QTX_INCLUDE_DIRS}
    ${LXQT_INCLUDE_DIRS}
)

set(H_FILES
    main.h
    monitorsettingsdialog.h
    monitor.h
    monitorwidget.h
    xrandr.h
)

set(CPP_FILES
    main.cpp
    monitorsettingsdialog.cpp
    xrandr.cpp
    monitor.cpp
    monitorwidget.cpp
    timeoutdialog.cpp
)

set(UI_FILES
    mainwindow.ui
    monitorwidget.ui
    timeoutdialog.ui
)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})

# Translations **********************************
include(LxQtTranslate)
lxqt_translate_ts(QM_FILES SOURCES
    ${H_FILES}
    ${CPP_FILES}
    ${UI_FILES}
    INSTALLATION_DIR "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_translate_desktop(DESKTOP_FILES SOURCES ${PROJECT_NAME}.desktop.in)

#************************************************

add_executable(${PROJECT_NAME} ${CPP_FILES} ${UI_FILES} ${RESOURCES} ${QRC_SOURCES} ${QM_FILES} ${MOC_SOURCES} ${DESKTOP_FILES})
target_link_libraries(${PROJECT_NAME} ${QTX_LIBRARIES} ${LXQT_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")
