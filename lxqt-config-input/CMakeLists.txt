project(lxqt-config-input)
find_package(X11 REQUIRED)

set(QTX_INCLUDE_DIRS  "")
set(QTX_LIBRARIES Qt5::Widgets Qt5::X11Extras)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")

include_directories(
    ${QTX_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
    ${LXQT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    "${CMAKE_SOURCE_DIR}/liblxqt-config-cursor"
    "${CMAKE_BINARY_DIR}/liblxqt-config-cursor"
)

set(lxqt-config-input_HDRS
    keyboardconfig.h
    mouseconfig.h
    mouseconfig.h
    keyboardlayoutconfig.h
    selectkeyboardlayoutdialog.h
)

set(lxqt-config-input_SRCS
    lxqt-config-input.cpp
    keyboardconfig.cpp
    mouseconfig.cpp
    keyboardlayoutconfig.cpp
    selectkeyboardlayoutdialog.cpp
)

set(lxqt-config-input_UIS
    mouseconfig.ui
    keyboardconfig.ui
    keyboardlayoutconfig.ui
    selectkeyboardlayoutdialog.ui
)

qt5_wrap_ui(lxqt-config-input_UI_H ${lxqt-config-input_UIS})

# Translations **********************************
include(LxQtTranslate)
lxqt_translate_ts(QM_FILES SOURCES
    ${lxqt-config-input_HDRS}
    ${lxqt-config-input_SRCS}
    ${lxqt-config-input_UIS}
    INSTALLATION_DIR "${LXQT_TRANSLATIONS_DIR}/${PROJECT_NAME}"
)

lxqt_translate_desktop(DESKTOP_FILES SOURCES ${PROJECT_NAME}.desktop.in)

#************************************************

add_definitions(
  -DPACKAGE_DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/lxqt-config-input"
)

add_executable(lxqt-config-input ${lxqt-config-input_SRCS} ${lxqt-config-input_UI_H} ${lxqt-config-input_MOCS} ${DESKTOP_FILES} ${QM_FILES})
target_link_libraries(lxqt-config-input ${QTX_LIBRARIES} ${X11_LIBRARIES} ${LXQT_LIBRARIES} lxqt-config-cursor)

install(TARGETS lxqt-config-input RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")
