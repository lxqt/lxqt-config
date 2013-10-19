cmake_minimum_required(VERSION 2.6)
# additional cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package(LXQT REQUIRED)
include(${LXQT_USE_FILE})

find_package(QTXDG REQUIRED)
include(${QTXDG_USE_FILE})

add_subdirectory(src)
add_subdirectory(lxqt-config-mouse)
add_subdirectory(lxqt-config-appearance)
add_subdirectory(lxqt-config-qtconfig)
add_subdirectory(lxqt-config-file-associations)


