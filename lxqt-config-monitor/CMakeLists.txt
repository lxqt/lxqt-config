project(lxqt-config-monitor)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories (
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${LXQT_INCLUDE_DIRS}
)

find_package(KF5Screen REQUIRED)

set(H_FILES
    monitorsettingsdialog.h
    monitor.h
    monitorwidget.h
    monitorpicture.h
)

set(CPP_FILES
    main.cpp
    monitorsettingsdialog.cpp
    monitor.cpp
    monitorwidget.cpp
    timeoutdialog.cpp
    monitorpicture.cpp
)

set(UI_FILES
    monitorsettingsdialog.ui
    monitorwidget.ui
    timeoutdialog.ui
    monitorpicture.ui
)

set(ICON_FILES
	icons/monitor.svg
	icons/extended.svg
	icons/monitor1onmonitor2ff.svg
	icons/unified.svg
	icons/monitor1offmonitor2on.svg
)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})

# Translations **********************************
lxqt_translate_ts(QM_FILES
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${H_FILES}
        ${CPP_FILES}
        ${UI_FILES}
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_app_translation_loader(QM_LOADER ${PROJECT_NAME})
lxqt_translate_desktop(DESKTOP_FILES SOURCES ${PROJECT_NAME}.desktop.in)

#************************************************

add_executable(${PROJECT_NAME}
    ${CPP_FILES}
    ${RESOURCES}
    ${QRC_SOURCES}
    ${QM_FILES}
    ${DESKTOP_FILES}
    ${QM_LOADER}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
    Qt5::X11Extras
    KF5::Screen
    ${LXQT_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")
install(FILES ${ICON_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/lxqt/icons/")
