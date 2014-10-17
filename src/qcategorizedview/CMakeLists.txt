project(qcategorizedview)

set(QTX_INCLUDE_DIRS  "")
set(QTX_LIBRARIES Qt5::Widgets)

set(SOURCES
    qcategorizedview.cpp
    qcategorydrawer.cpp
    qcategorizedsortfilterproxymodel.cpp
)

include_directories(
    ${QTX_INCLUDE_DIRS}
    "${CMAKE_CURRENT_BINARY_DIR}"
)

add_library(qcategorizedview STATIC ${SOURCES} ${MOC_SOURCES})
target_link_libraries(qcategorizedview ${QTX_LIBRARIES})
