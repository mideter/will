find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(will-gui
  main.cpp
  mainwindow.cpp
  mainwindow.h
  willchatbridge.cpp
  willchatbridge.h
  ${CMAKE_SOURCE_DIR}/src/client/messengerclient.cpp
)

target_compile_features(will-gui PRIVATE cxx_std_20)

target_include_directories(will-gui PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/client
)

target_link_libraries(will-gui PRIVATE Qt5::Widgets common)
