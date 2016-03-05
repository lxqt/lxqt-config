project(lxqt-config-brightness-settings)

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

set(DESKTOP_FILES
    brightnesssettings.desktop
)

qt5_wrap_ui(UI_HEADERS ${UI_FILES})


add_executable(${PROJECT_NAME} ${CPP_FILES} ${UI_FILES} ${RESOURCES} ${QRC_SOURCES} ${QM_FILES} ${MOC_SOURCES} ${DESKTOP_FILES})

target_link_libraries(
    ${PROJECT_NAME}
    ${QTX_LIBRARIES}
    ${XRANDR_LIBRARIES}
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
    
