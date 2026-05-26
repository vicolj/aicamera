cmake_minimum_required(VERSION 3.16)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(edger-qt-config
  main.cpp
  main_window.cpp
  roi_editor_widget.cpp
)

target_include_directories(edger-qt-config PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(edger-qt-config PRIVATE
  edger_core
  Qt${QT_VERSION_MAJOR}::Widgets
)

set_target_properties(edger-qt-config PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS edger-qt-config RUNTIME DESTINATION bin)
