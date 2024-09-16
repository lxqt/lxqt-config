project(lxqt-monitor)

find_package(UDev REQUIRED)
include_directories(${LIBUDEV_INCLUDE_DIR})
add_definitions(-DWITH_LIBUDEV_MONITOR)

set(HEADERS
    Daemon.h
    UdevNotifier.h
)

set(SOURCES
    main.cpp
    Daemon.cpp
    UdevNotifier.cpp
)

set(UIS

)

#set(ICONS
#)

# Config file
#configure_file(resources/configure.in configure.h)

# Translations **********************************
#lxqt_translate_ts(QM_FILES
#    UPDATE_TRANSLATIONS
#        ${UPDATE_TRANSLATIONS}
#    SOURCES
#        ${HEADERS}
#        ${SOURCES}
#        ${UIS}
#    INSTALL_DIR
#        "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
#)
#
#lxqt_app_translation_loader(QM_LOADER ${PROJECT_NAME})
#lxqt_translate_desktop(DESKTOP_FILES SOURCES "resources/${PROJECT_NAME}.desktop.in")

#************************************************

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${RESOURCES}
    ${QRC_SOURCES}
    ${DESKTOP_FILES}
    ${QM_FILES}
    ${QM_LOADER}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
    KF5::WindowSystem
    Qt5::DBus
    ${UDEV_LIBS}
    lxqt
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
install(FILES ${DESKTOP_FILES} DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/applications")
#install(FILES ${ICONS} DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/lxqt/icons")

