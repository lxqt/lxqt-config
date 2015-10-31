project(lxqt-config-monitor-daemon)

set(HEADERS
    daemon.h
)

set(SOURCES
    main.cpp
    daemon.cpp
)

# Translations **********************************
lxqt_translate_ts(QM_FILES
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${HEADERS}
        ${SOURCES}
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_app_translation_loader(QM_LOADER ${PROJECT_NAME})

#************************************************

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${QM_FILES}
    ${QM_LOADER}
)

target_link_libraries(${PROJECT_NAME}
    KF5::Screen
    lxqt
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
