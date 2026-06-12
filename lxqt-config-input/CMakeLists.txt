find_package(X11 REQUIRED)

if (WITH_TOUCHPAD)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XORG_LIBINPUT REQUIRED xorg-libinput)
    pkg_check_modules(LIBUDEV REQUIRED libudev)
endif ()

set(lxqt-config-input_HDRS
    keyboardconfig.h
    mouseconfig.h
    mouseconfig.h
    keyboardlayoutconfig.h
    selectkeyboardlayoutdialog.h
    ../lxqt-config-appearance/configothertoolkits.h
)

set(lxqt-config-input_SRCS
    lxqt-config-input.cpp
    keyboardconfig.cpp
    mouseconfig.cpp
    keyboardlayoutconfig.cpp
    selectkeyboardlayoutdialog.cpp
    ../lxqt-config-appearance/configothertoolkits.cpp
)

set(lxqt-config-input_UIS
    mouseconfig.ui
    keyboardconfig.ui
    keyboardlayoutconfig.ui
    selectkeyboardlayoutdialog.ui
)

if (WITH_TOUCHPAD)
    set(lxqt-config-input_HDRS
        ${lxqt-config-input_HDRS}
        touchpadconfig.h
    )

    set(lxqt-config-input_SRCS
        ${lxqt-config-input_SRCS}
        touchpadconfig.cpp
        touchpaddevice.cpp
    )

    set(lxqt-config-input_UIS
        ${lxqt-config-input_UIS}
        touchpadconfig.ui
    )
endif ()

# Translations **********************************
lxqt_translate_ts(QM_FILES
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${lxqt-config-input_HDRS}
        ${lxqt-config-input_SRCS}
        ${lxqt-config-input_UIS}
    TEMPLATE
        lxqt-config-input 
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/lxqt-config-input"
)

lxqt_app_translation_loader(QM_LOADER lxqt-config-input)
lxqt_translate_desktop(DESKTOP_FILES
    SOURCES
        lxqt-config-input.desktop.in
        resources/lxqt-config-touchpad-autostart.desktop.in
    USE_YAML
)

#************************************************

add_executable(lxqt-config-input
    ${lxqt-config-input_SRCS}
    ${DESKTOP_FILES}
    ${QM_FILES}
    ${QM_LOADER}
)

target_compile_definitions(lxqt-config-input
    PRIVATE
        "$<$<BOOL:${WITH_TOUCHPAD}>:-DWITH_TOUCHPAD=ON>"
        "-DPACKAGE_DATA_DIR=\"${CMAKE_INSTALL_FULL_DATAROOTDIR}/lxqt-config-input\""
)

target_include_directories(lxqt-config-input
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../liblxqt-config-cursor"
        "$<$<BOOL:${WITH_TOUCHPAD}>:${XORG_LIBINPUT_INCLUDE_DIRS}>"
)

target_link_libraries(lxqt-config-input
    Qt6::Widgets
    X11::X11
    lxqt
    lxqt-config-cursor
    $<$<BOOL:${WITH_TOUCHPAD}>:${X11_Xinput_LIB}>
    $<$<BOOL:${WITH_TOUCHPAD}>:udev>
)

set_target_properties(lxqt-config-input
    PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}/${LXQT_CONFIG_PROJECT}"
)

install(TARGETS
    lxqt-config-input
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT Runtime
)
install(FILES
    ${DESKTOP_FILES}
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
    COMPONENT Runtime
)
