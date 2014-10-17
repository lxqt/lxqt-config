project(lxqt-config-file-associations)

set(QTX_INCLUDE_DIRS  "")
set(QTX_LIBRARIES Qt5::Widgets Qt5::Xml Qt5::Xml Qt5::DBus Qt5::Concurrent)

include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
)

add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")

set(H_FILES
    mimetypedata.h
    mimetypeviewer.h
    applicationchooser.h
)

set(MOC_FILES
    mimetypedata.h
    mimetypeviewer.h
    applicationchooser.h
)

set(CPP_FILES
    mimetypedata.cpp
    mimetypeviewer.cpp
    applicationchooser.cpp
    main.cpp
)

set(UI_FILES
    mimetypeviewer.ui
    applicationchooser.ui
)

set(QRC_FILES "")

set(LIBRARIES
    ${QTXDG_LIBRARIES}
    ${LXQT_LIBRARIES}
)

# Translations **********************************
include(LxQtTranslate)
lxqt_translate_ts(QM_FILES SOURCES
    ${H_FILES}
    ${CPP_FILES}
    ${UI_FILES}
    INSTALLATION_DIR "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_translate_desktop(DESKTOP_FILES SOURCES ${PROJECT_NAME}.desktop.in)

#************************************************

qt5_wrap_ui(UI_HEADERS ${UI_FILES})
qt5_add_resources(QRC_SOURCES ${QRC_FILES})

add_executable(${PROJECT_NAME} ${CPP_FILES} ${UI_FILES} ${RESOURCES} ${QRC_SOURCES} ${QM_FILES} ${MOC_SOURCES} ${DESKTOP_FILES})
target_link_libraries(${PROJECT_NAME}  ${LIBRARIES} ${QTX_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
