cmake_minimum_required(VERSION 3.14)
project(will LANGUAGES CXX)

# Единая папка для всех исполняемых файлов (в т.ч. из add_subdirectory).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(common
  src/common/sockethandle.cpp
  src/common/ipv4.cpp
  src/common/port.cpp
  src/common/serveraddress.cpp
  src/common/willprotocol.cpp
  src/common/willmessage.cpp
)
target_include_directories(common PUBLIC src/common)

add_executable(will-server
  src/server/server.cpp
  src/server/clienthub.cpp
  src/server/messengerloops.cpp
  src/server/messengerserver.cpp
  src/server/listensocketstopsignals.cpp
  src/server/connectionacceptor.cpp
  src/server/client.cpp
  src/server/clientconnection.cpp
  src/server/clientaddress.cpp
)
target_include_directories(will-server PRIVATE src/server)
target_link_libraries(will-server PRIVATE common)

add_executable(will-client
  src/client/client.cpp
  src/client/chatsession.cpp
  src/client/messengerclient.cpp
)
target_include_directories(will-client PRIVATE src/client)
target_link_libraries(will-client PRIVATE common)

find_package(Qt5 QUIET COMPONENTS Widgets)
if(Qt5Widgets_FOUND)
  add_subdirectory(src/gui)
endif()

enable_testing()
add_executable(will-message-test tests/will_message_test.cpp)
target_link_libraries(will-message-test PRIVATE common)
add_test(NAME will-message COMMAND will-message-test)

