project(lxqt-config-monitor-daemon)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories (
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${LXQT_INCLUDE_DIRS}
)

find_package(KF5Screen REQUIRED)
find_package(Qt5Core REQUIRED)

set(H_FILES
    screen_listener.h
)

set(CPP_FILES
    screen_listener.cpp
    main.cpp
)



qt5_wrap_ui(UI_HEADERS ${UI_FILES})

# Translations **********************************
lxqt_translate_ts(QM_FILES
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${H_FILES}
        ${CPP_FILES}
        ${UI_FILES}
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_app_translation_loader(QM_LOADER ${PROJECT_NAME})

#************************************************

add_executable(${PROJECT_NAME}
    ${CPP_FILES}
    ${UI_FILES}
    ${RESOURCES}
    ${QRC_SOURCES}
    ${QM_FILES}
    ${MOC_SOURCES}
    ${DESKTOP_FILES}
    ${QM_LOADER}
)

target_link_libraries(${PROJECT_NAME}
    KF5::Screen
    ${LXQT_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
