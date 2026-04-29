cmake_minimum_required(VERSION 3.14)
project(will LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(common
  src/common/sockethandle.cpp
  src/common/ipv4.cpp
  src/common/port.cpp
  src/common/serveraddress.cpp
)
target_include_directories(common PUBLIC src/common)

add_executable(will-server
  src/server/server.cpp
  src/server/messengerserver.cpp
  src/server/clientconnection.cpp
  src/server/clientaddress.cpp
)
target_include_directories(will-server PRIVATE src/server)
target_link_libraries(will-server PRIVATE common)

add_executable(will-client
  src/client/client.cpp
  src/client/messengerclient.cpp
)
target_include_directories(will-client PRIVATE src/client)
target_link_libraries(will-client PRIVATE common)

