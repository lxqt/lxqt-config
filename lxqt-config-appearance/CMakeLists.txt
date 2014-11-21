project(lxqt-config-appearance)

include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${LXQT_INCLUDE_DIRS}
    ${QTXDG_INCLUDE_DIRS}
    "${CMAKE_SOURCE_DIR}/liblxqt-config-cursor"
    "${CMAKE_BINARY_DIR}/liblxqt-config-cursor"
)

set(H_FILES
    iconthemeinfo.h
)

set(MOC_FILES
    iconthemeconfig.h
    lxqtthemeconfig.h
    fontsconfig.h
    styleconfig.h
    fontconfigfile.h
)

set(CPP_FILES
    main.cpp
    iconthemeconfig.cpp
    iconthemeinfo.cpp
    lxqtthemeconfig.cpp
    fontsconfig.cpp
    styleconfig.cpp
    fontconfigfile.cpp
)

set(UI_FILES
    iconthemeconfig.ui
    lxqtthemeconfig.ui
    fontsconfig.ui
    styleconfig.ui
)

set(QRC_FILES "")

set(LIBRARIES
    ${QTXDG_LIBRARIES}
    ${LXQT_LIBRARIES}
)


# Translations **********************************
lxqt_translate_ts(QM_FILES
        UPDATE_TRANSLATIONS ${UPDATE_TRANSLATIONS}
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

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_add_resources(QRC_SOURCES ${QRC_FILES})

add_executable(${PROJECT_NAME}
    ${CPP_FILES}
    ${UI_FILES}
    ${RESOURCES}
    ${QRC_SOURCES}
    ${QM_FILES}
    ${QM_LOADER}
    ${MOC_SOURCES}
    ${DESKTOP_FILES}
)

target_link_libraries(${PROJECT_NAME}
    KF5::WindowSystem
    Qt5::Widgets
    Qt5::X11Extras
    Qt5::Xml
    ${LIBRARIES}
    lxqt-config-cursor
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
