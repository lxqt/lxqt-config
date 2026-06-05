set(H_FILES
    localeconfig.h
)


set(CPP_FILES
    main.cpp
    localeconfig.cpp
)

set(UI_FILES
    localeconfig.ui
)

qt6_wrap_cpp(COMBO_HEADER combobox.h)

# Translations **********************************
lxqt_translate_ts(QM_FILES
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${H_FILES}
        ${CPP_FILES}
        ${UI_FILES}
    TEMPLATE
        lxqt-config-locale
    INSTALL_DIR
        "${LXQT_TRANSLATIONS_DIR}/lxqt-config-locale"
)

lxqt_app_translation_loader(QM_LOADER lxqt-config-locale)
lxqt_translate_desktop(DESKTOP_FILES SOURCES lxqt-config-locale.desktop.in USE_YAML)

#************************************************

add_executable(lxqt-config-locale
    ${CPP_FILES}
    ${COMBO_HEADER}
    ${QM_FILES}
    ${DESKTOP_FILES}
    ${QM_LOADER}
)

target_link_libraries(lxqt-config-locale
    Qt6::Widgets
    Qt6::Xml
    lxqt
)

install(TARGETS
    lxqt-config-locale
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT Runtime
)
install(FILES
    ${DESKTOP_FILES}
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
    COMPONENT Runtime
)
