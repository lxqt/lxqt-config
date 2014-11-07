project(qcategorizedview)

set(SOURCES
    qcategorizedview.cpp
    qcategorydrawer.cpp
    qcategorizedsortfilterproxymodel.cpp
)

include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
)

add_library(qcategorizedview STATIC ${SOURCES} ${MOC_SOURCES})
target_link_libraries(qcategorizedview Qt5::Widgets)
