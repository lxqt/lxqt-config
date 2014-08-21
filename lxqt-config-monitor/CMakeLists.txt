project(lxqt-config-monitor)

cmake_minimum_required(VERSION 2.8.6)


set(H_FILES
	main.h  monitorsettingsdialog.h monitor.h xrandr.h
)

set(MOC_FILES
	main.h  monitorsettingsdialog.h monitor.h xrandr.h
)

set(CPP_FILES
  main.cpp
  monitorsettingsdialog.cpp
  xrandr.cpp
  monitor.cpp
)


set(UI_FILES
  mainwindow.ui
  monitor.ui
)

set(DESKTOP_FILES
         ${PROJECT_NAME}.desktop
)

######################################


option(USE_QT5 "Force use the Qt5." $ENV{USE_QT5})
option(USE_QT4 "Force use the Qt4." $ENV{USE_QT4})

if((USE_QT4 AND USE_QT5) OR
   (NOT USE_QT4 AND NOT USE_QT5))

    # Autodetect Qt version
    find_package(Qt4 QUIET)
    if (QT4_FOUND)
        set(USE_QT4 ON)
        set(USE_QT5 OFF)
    else()
        set(USE_QT4 OFF)
        set(USE_QT5 ON)
    endif()
endif()

if(USE_QT5)
    find_package(Qt5Widgets REQUIRED QUIET)
    # find_package(Qt5X11Extras REQUIRED QUIET)

    qt5_wrap_cpp(MOC_SOURCES ${MOC_FILES})
    qt5_wrap_ui(UI_HEADERS ${UI_FILES})
    qt5_add_resources(QRC_SOURCES ${QRC_FILES})

    set(QTX_LIBRARIES ${Qt5Widgets_LIBRARIES} ${Qt5X11Extras_LIBRARIES})
else()
    find_package(Qt4 REQUIRED QtCore QtGui QUIET)

    qt4_wrap_cpp(MOC_SOURCES ${MOC_FILES})
    qt4_wrap_ui(UI_HEADERS ${UI_FILES})
    qt4_add_resources(QRC_SOURCES ${QRC_FILES})

    set(QTX_LIBRARIES ${QT_LIBRARIES})
endif()


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_INCLUDE_DIR}
    ${LXQT_INCLUDE_DIR}
)


if(NOT CMAKE_BUILD_TYPE)
    set( CMAKE_BUILD_TYPE Release )
endif (NOT CMAKE_BUILD_TYPE)

set(APP_SHARE_DIR share/lxqt/${PROJECT_NAME})

#************************************************


# Translations **********************************
#lxqt_translate_desktop(DESKTOP_FILES
#    SOURCES
#        resources/lxqt-about.desktop.in
#)
#************************************************


add_executable(${PROJECT_NAME}
    ${CPP_FILES}
    ${UI_FILES}
    ${RESOURCES}
    ${QRC_SOURCES}
    ${QM_FILES}
    ${MOC_SOURCES}
)
#add_dependencies(${PROJECT_NAME} ${LXQT_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${QTX_LIBRARIES} ${LXQT_LIBRARIES})
if(USE_QT4)
	target_link_libraries(${PROJECT_NAME} Qt4::QtGui Qt4::QtXml)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${CONFIG_FILES} DESTINATION ${APP_SHARE_DIR})
install(FILES ${DESKTOP_FILES} DESTINATION share/applications)
install(FILES ${ICON_FILES} DESTINATION share/icons)
install(FILES ${TRANSLATIONS_FILES} DESTINATION ${APP_SHARE_DIR}/translations-qm)

# building tarball with CPack -------------------------------------------------
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_VERSION_MAJOR ${LXQT_MAJOR_VERSION})
set (CPACK_PACKAGE_VERSION_MINOR ${LXQT_MINOR_VERSION})
set (CPACK_PACKAGE_VERSION_PATCH ${LXQT_PATCH_VERSION})
set (CPACK_GENERATOR TBZ2)
set (CPACK_SOURCE_GENERATOR TBZ2)
set (CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.git;.kdev4;temp)
include (CPack)


######################################

