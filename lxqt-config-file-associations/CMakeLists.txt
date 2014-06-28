project(lxqt-config-file-associations)

if(USE_QT5)
  set(QTX_INCLUDE_DIRS  "")
  set(QTX_LIBRARIES Qt5::Widgets Qt5::Xml Qt5::Xml Qt5::DBus Qt5::Concurrent)
else()
  set(QTX_INCLUDE_DIRS  ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTDBUS_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR})
  set(QTX_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTDBUS_LIBRARY})
endif()

include_directories (
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QTX_INCLUDE_DIRS}
    ${LXQT_INCLUDE_DIRS}
    ${QTXDG_INCLUDE_DIRS}
    )

set(H_FILES
    mimetypeviewer.h
    applicationchooser.h
    mimetypeitemmodel.h
    busyindicator.h
)

set(MOC_FILES
    mimetypeviewer.h
    applicationchooser.h
    busyindicator.h
)

set(CPP_FILES
    mimetypeviewer.cpp
    applicationchooser.cpp
    mimetypeitemmodel.cpp
    busyindicator.cpp
    main.cpp
)

set(UI_FILES
    mimetypeviewer.ui
    applicationchooser.ui
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

if(USE_QT5)
    qt5_wrap_ui(UI_HEADERS ${UI_FILES})
    qt5_add_resources(QRC_SOURCES ${QRC_FILES})
else()
#    qt4_wrap_cpp(MOC_SOURCES ${MOC_FILES})
    qt4_wrap_ui(UI_HEADERS ${UI_FILES})
    qt4_add_resources(QRC_SOURCES ${QRC_FILES})
endif()

add_executable(${PROJECT_NAME} ${CPP_FILES} ${UI_FILES} ${RESOURCES} ${QRC_SOURCES} ${QM_FILES} ${MOC_SOURCES} ${DESKTOP_FILES})
target_link_libraries(${PROJECT_NAME}  ${LIBRARIES} ${QTX_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
