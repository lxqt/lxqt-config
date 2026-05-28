find_package(KF6Screen ${KF6SCREEN_MINIMUM_VERSION} REQUIRED)
find_package(Qt6Svg ${QT_MINIMUM_VERSION} REQUIRED)
find_package(Qt6SvgWidgets ${QT_MINIMUM_VERSION} REQUIRED)

set(HEADERS
    monitorsettingsdialog.h
    monitor.h
    monitorwidget.h
    monitorpicture.h
    loadsettings.h
    settingsdialog.h
    managesavedsettings.h
    fastmenu.h
    kscreenutils.h
)

set(SOURCES
    main.cpp
    monitorsettingsdialog.cpp
    monitor.cpp
    monitorwidget.cpp
    timeoutdialog.cpp
    monitorpicture.cpp
    loadsettings.cpp
    settingsdialog.cpp
    managesavedsettings.cpp
    fastmenu.cpp
    kscreenutils.cpp
)

set(UIS
    monitorsettingsdialog.ui
    monitorwidget.ui
    timeoutdialog.ui
    monitorpicture.ui
    managesavedsettings.ui
    fastmenu.ui
)

set(ICONS
    icons/monitor.svg
)

# Config file
configure_file(resources/configure.in configure.h)

# Translations **********************************
lxqt_translate_ts(QM_FILES
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${HEADERS}
        ${SOURCES}
        ${UIS}
    TEMPLATE
        lxqt-config-monitor
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/lxqt-config-monitor"
)

lxqt_app_translation_loader(QM_LOADER lxqt-config-monitor)
lxqt_translate_desktop(DESKTOP_FILES
    SOURCES
        "resources/lxqt-config-monitor.desktop.in"
        "resources/lxqt-config-monitor-autostart.desktop.in"
    USE_YAML
)

#************************************************

add_executable(lxqt-config-monitor
    ${SOURCES}
    ${RESOURCES}
    ${QRC_SOURCES}
    ${DESKTOP_FILES}
    ${QM_FILES}
    ${QM_LOADER}
)

target_link_libraries(lxqt-config-monitor
    Qt6::Widgets
    Qt6::Svg
    Qt6::SvgWidgets
    KF6::Screen
    lxqt
)

install(TARGETS lxqt-config-monitor RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
install(FILES ${DESKTOP_FILES} DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/applications")
install(FILES ${ICONS} DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/lxqt/icons")

