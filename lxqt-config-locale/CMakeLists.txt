project(lxqt-config-locale)

include_directories(
    ${QTXDG_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${LXQT_INCLUDE_DIRS}
    ${QTXDG_INCLUDE_DIRS}
)

set(H_FILES
    localeconfig.h
)

set(MOC_FILES
    localeconfig.h
)

set(CPP_FILES
    main.cpp
    localeconfig.cpp
)

set(UI_FILES
    localeconfig.ui
)

set(QRC_FILES "")

set(LIBRARIES
    ${QTXDG_LIBRARIES}
    ${LXQT_LIBRARIES}
)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(FATAL "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. C++11 support is required")
endif()

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
    Qt5::Widgets
    Qt5::Xml
    ${LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
