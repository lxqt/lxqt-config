project(lxqt-config-input)
find_package(X11 REQUIRED)

# set(CMAKE_CXX_FLAGS "-DQT_NO_KEYWORDS -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

include_directories(
  ${QT_INCLUDES}
  ${X11_INCLUDE_DIR}
  ${LXQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/lxqt-config-cursor
  ${CMAKE_BINARY_DIR}/lxqt-config-cursor
)

set(lxqt-config-input_HDRS
  keyboardconfig.h
  mouseconfig.h
)

set(lxqt-config-input_SRCS
  lxqt-config-input.cpp
  keyboardconfig.cpp
  mouseconfig.cpp
)

set(lxqt-config-input_UIS
  mouseconfig.ui
  keyboardconfig.ui
)

qt4_wrap_ui(lxqt-config-input_UI_H ${lxqt-config-input_UIS})
qt4_wrap_cpp(lxqt-config-input_MOCS ${lxqt-config-input_HDRS})

# Translations **********************************
include(LxQtTranslate)
lxqt_translate_ts(QM_FILES
    SOURCES
        ${lxqt-config-input_HDRS}
        ${lxqt-config-input_SRCS}
        ${lxqt-config-input_UIS}
    INSTALLATION_DIR
        ${CMAKE_INSTALL_PREFIX}/share/lxqt/lxqt-config
)

#************************************************

add_definitions(
  -DPACKAGE_DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/lxqt-config-input"
)

add_executable(lxqt-config-input ${lxqt-config-input_SRCS} ${lxqt-config-input_UI_H} ${lxqt-config-input_MOCS} ${DESKTOP_FILES} ${QM_FILES})
add_dependencies(lxqt-config-input lxqt)
target_link_libraries( lxqt-config-input  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${X11_LIBRARIES} ${LXQT_LIBRARIES} lxqt-config-mouse)

INSTALL(TARGETS lxqt-config-input RUNTIME DESTINATION bin)
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
