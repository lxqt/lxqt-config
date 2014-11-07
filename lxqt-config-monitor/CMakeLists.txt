project(lxqt-config-monitor)

include_directories (
    "${CMAKE_CURRENT_BINARY_DIR}"
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
lxqt_translate_ts(QM_FILES SOURCES
    UPDATE_TRANSLATIONS ${UPDATE_TRANSLATIONS}
    ${H_FILES}
    ${CPP_FILES}
    ${UI_FILES}
    INSTALL_DIR "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_app_translation_loader(QM_LOADER ${PROJECT_NAME})
lxqt_translate_desktop(DESKTOP_FILES SOURCES ${PROJECT_NAME}.desktop.in)

#************************************************

add_executable(${PROJECT_NAME} ${CPP_FILES} ${UI_FILES} ${RESOURCES} ${QRC_SOURCES} ${QM_FILES} ${MOC_SOURCES} ${DESKTOP_FILES} ${QM_LOADER})
target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
    Qt5::X11Extras
    ${LXQT_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")
