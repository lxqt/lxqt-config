cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(lxqt-config)
set(LXQT_CONFIG_PROJECT "${PROJECT_NAME}")

option(UPDATE_TRANSLATIONS "Update source translation translations/*.ts files" OFF)
option(WITH_INPUT "Build the 'lxqt-config-input'" ON)
option(WITH_FILE_ASSOCIATIONS "Build the 'lxqt-config-file-associations'" ON)
option(WITH_APPEARANCE "Build the 'lxqt-config-appearance'" ON)
option(WITH_MONITOR "Build the 'lxqt-config-monitor'" ON)
option(WITH_BRIGHTNESS "Build the 'lxqt-config-brightness'" ON)
option(WITH_LOCALE "Build the 'lxqt-config-locale'" ON)

include(GNUInstallDirs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5X11Extras REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(lxqt REQUIRED)

include(LXQtCompilerSettings NO_POLICY_SCOPE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Patch Version
set(LXQT_CONFIG_PATCH_VERSION 0)

set(LXQT_CONFIG_VERSION ${LXQT_MAJOR_VERSION}.${LXQT_MINOR_VERSION}.${LXQT_CONFIG_PATCH_VERSION})
add_definitions(
    "-DLXQT_CONFIG_VERSION=\"${LXQT_CONFIG_VERSION}\""
    "-DQT_NO_FOREACH"
)

include(LXQtTranslate)

add_subdirectory(src)
if (WITH_INPUT OR WITH_APPEARANCE)
    add_subdirectory(liblxqt-config-cursor)
endif()
if (WITH_INPUT)
    add_subdirectory(lxqt-config-input)
endif()
if (WITH_FILE_ASSOCIATIONS)
    add_subdirectory(lxqt-config-file-associations)
endif()
if (WITH_APPEARANCE)
    add_subdirectory(lxqt-config-appearance)
endif()
if (WITH_MONITOR)
    add_subdirectory(lxqt-config-monitor)
endif()
if (WITH_BRIGHTNESS)
    add_subdirectory(lxqt-config-brightness)
endif()
if (WITH_LOCALE)
    add_subdirectory(lxqt-config-locale)
endif()
