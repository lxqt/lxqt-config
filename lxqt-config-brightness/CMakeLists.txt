find_package(XCB REQUIRED COMPONENTS XCB RANDR)

set(H_FILES
    brightnesswatcher.h
    brightnesssettings.h
    xrandrbrightness.h
    monitorinfo.h
    outputwidget.h
)

set(CPP_FILES
    main.cpp
    brightnesswatcher.cpp
    brightnesssettings.cpp
    xrandrbrightness.cpp
    monitorinfo.cpp
    outputwidget.cpp
)

set(UI_FILES
    brightnesssettings.ui
    outputwidget.ui
)

qt6_wrap_ui(UI_HEADERS ${UI_FILES})

# Translations **********************************
lxqt_translate_ts(QM_FILES
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${H_FILES}
        ${CPP_FILES}
        ${UI_FILES}
    TEMPLATE
        lxqt-config-brightness
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/lxqt-config-brightness"
)

lxqt_app_translation_loader(QM_LOADER lxqt-config-brightness)
lxqt_translate_desktop(DESKTOP_FILES SOURCES "resources/lxqt-config-brightness.desktop.in" USE_YAML)

add_executable(lxqt-config-brightness ${CPP_FILES} ${UI_FILES} ${QM_FILES} ${DESKTOP_FILES} ${QM_LOADER})

target_link_libraries(lxqt-config-brightness
    Qt6::Widgets
    XCB::XCB
    XCB::RANDR
    lxqt
)

target_compile_definitions(lxqt-config-brightness
    PRIVATE
        -DICON_DIR="${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/hicolor/48x48/apps"
)

install(TARGETS lxqt-config-brightness RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(
    FILES icons/brightnesssettings.svg
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps"
)

