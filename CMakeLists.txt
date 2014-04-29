cmake_minimum_required(VERSION 2.6)

project(lxqt-config)

# additional cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Set default installation paths
set(LIB_INSTALL_DIR "lib${LIB_SUFFIX}" CACHE PATH "Installation path for libraries")

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package(LXQT REQUIRED)
include(${LXQT_USE_FILE})

find_package(QTXDG REQUIRED)
include(${QTXDG_USE_FILE})

add_subdirectory(src)
add_subdirectory(lxqt-config-cursor)
add_subdirectory(lxqt-config-appearance)
add_subdirectory(lxqt-config-qtconfig)
add_subdirectory(lxqt-config-file-associations)
add_subdirectory(lxqt-config-input)

# building tarball with CPack -------------------------------------------------
include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_VERSION_MAJOR ${LXQT_MAJOR_VERSION})
set (CPACK_PACKAGE_VERSION_MINOR ${LXQT_MINOR_VERSION})
set (CPACK_PACKAGE_VERSION_PATCH ${LXQT_PATCH_VERSION})
set (CPACK_GENERATOR TBZ2)
set (CPACK_SOURCE_GENERATOR TBZ2)
set (CPACK_SOURCE_IGNORE_FILES /build/;.gitignore;.*~;.git;.kdev4;temp)
include (CPack)
