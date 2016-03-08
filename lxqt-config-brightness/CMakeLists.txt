project(lxqt-config-brightness)

find_package(Qt5Widgets REQUIRED QUIET)
find_package(Qt5X11Extras REQUIRED QUIET)

find_package(PkgConfig)

pkg_check_modules(XRANDR xcb-randr)

set(QTX_INCLUDE_DIRS  "")
set(QTX_LIBRARIES Qt5::Widgets Qt5::X11Extras)


include_directories (
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QTX_INCLUDE_DIRS}
    ${XRANDR_INCLUDE_DIRS}
)

set(H_FILES
    brightnesssettings.h
	xrandrbrightness.h
    monitorinfo.h
    outputwidget.h
)

set(CPP_FILES
    main.cpp
    brightnesssettings.cpp
    xrandrbrightness.cpp
    monitorinfo.cpp
    outputwidget.cpp
)

set(UI_FILES
    brightnesssettings.ui
    outputwidget.ui
)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})


# Translations **********************************
lxqt_translate_ts(QM_FILES
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${HEADERS}
        ${SOURCES}
        ${UIS}
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_app_translation_loader(QM_LOADER ${PROJECT_NAME})
lxqt_translate_desktop(DESKTOP_FILES SOURCES "resources/${PROJECT_NAME}.desktop.in")


add_executable(${PROJECT_NAME} ${CPP_FILES} ${UI_FILES} ${RESOURCES} ${QRC_SOURCES} ${QM_FILES} ${MOC_SOURCES} ${DESKTOP_FILES})

target_link_libraries(
    ${PROJECT_NAME}
    ${QTX_LIBRARIES}
    ${XRANDR_LIBRARIES}
    lxqt
)

add_definitions(
  -DICON_DIR="${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps"
)


install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
install(
    FILES icons/brightnesssettings.svg
    DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps"
)
    
