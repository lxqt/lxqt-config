project(lxqt-config-appearance)

include_directories (
    ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
    ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTDBUS_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR}
    ${LXQT_INCLUDE_DIRS}
    ${QTXDG_INCLUDE_DIRS}

    ${CMAKE_SOURCE_DIR}/lxqt-config-cursor
    ${CMAKE_BINARY_DIR}/lxqt-config-cursor
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

set(QRC_FILES

)

set(LIBRARIES
    ${QTXDG_LIBRARIES}
    ${LXQT_LIBRARIES}
)


# Translations **********************************
include(LxQtTranslate)
lxqt_translate_ts(QM_FILES 
    SOURCES
        ${H_FILES}
        ${CPP_FILES} 
        ${UI_FILES}
    INSTALLATION_DIR
        ${CMAKE_INSTALL_PREFIX}/share/lxqt/lxqt-config
)

lxqt_translate_desktop(DESKTOP_FILES
    SOURCES
        ${PROJECT_NAME}.desktop.in
) 

#************************************************

qt4_wrap_cpp(MOC_SOURCES ${MOC_FILES})
qt4_wrap_ui(UI_HEADERS ${UI_FILES})
qt4_add_resources(QRC_SOURCES ${QRC_FILES})


add_executable(${PROJECT_NAME} ${CPP_FILES} ${UI_FILES} ${RESOURCES} ${QRC_SOURCES} ${QM_FILES} ${MOC_SOURCES} ${DESKTOP_FILES})
target_link_libraries(${PROJECT_NAME}  ${LIBRARIES} ${QT_LIBRARIES} ${QT_QTXML_LIBRARY} lxqt-config-mouse)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
