project(qcategorizedview)

if(USE_QT5)
  set(QTX_INCLUDE_DIRS  "")
  set(QTX_LIBRARIES Qt5::Widgets)
else()
  set(QTX_INCLUDE_DIRS  ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_HEADERS_DIR})
  set(QTX_LIBRARIES ${QT_QTGUI_LIBRARIES} ${QT_QTCORE_LIBRARIES})
endif()

set(SOURCES
  qcategorizedview.cpp
  qcategorydrawer.cpp
  qcategorizedsortfilterproxymodel.cpp
)

include_directories(
  ${QTX_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(qcategorizedview STATIC ${SOURCES} ${MOC_SOURCES})
target_link_libraries(qcategorizedview ${QTX_LIBRARIES})
