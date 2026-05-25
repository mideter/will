cmake_minimum_required(VERSION 3.14)
project(will LANGUAGES CXX)

# Single output directory for all executables (including from add_subdirectory).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-30-2
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(asio)

FetchContent_Declare(
  cli11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.5.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(cli11)

find_package(Threads REQUIRED)

add_library(common
  src/common/hostaddress.cpp
  src/common/willprotocol.cpp
  src/common/willmessage.cpp
)
target_include_directories(common PUBLIC
  src/common
  ${asio_SOURCE_DIR}/asio/include
)
target_compile_definitions(common PUBLIC ASIO_STANDALONE)

add_executable(will-server
  src/server/server.cpp
  src/server/cli/servercliapp.cpp
  src/server/cli/serverconfigparser.cpp
  src/server/sessionregistry.cpp
  src/server/serverconfigvalidator.cpp
  src/server/willserver.cpp
  src/server/asioserver.cpp
  src/server/session.cpp
  src/server/tcpframereader.cpp
)
target_include_directories(will-server PRIVATE
  src/server
  src/server/cli
  ${asio_SOURCE_DIR}/asio/include
)
target_compile_definitions(will-server PRIVATE ASIO_STANDALONE)
target_link_libraries(will-server PRIVATE common CLI11::CLI11 Threads::Threads)

add_executable(will-client
  src/client/client.cpp
  src/client/chatsession.cpp
  src/client/willclient.cpp
  src/client/clientconfigvalidator.cpp
  src/client/cli/clientcliapp.cpp
  src/client/cli/clientconfigparser.cpp
)
target_include_directories(will-client PRIVATE
  src/client
  src/client/cli
  ${asio_SOURCE_DIR}/asio/include
)
target_compile_definitions(will-client PRIVATE ASIO_STANDALONE)
target_link_libraries(will-client PRIVATE common CLI11::CLI11 Threads::Threads)

add_executable(will-load-clients
  tools/load_clients.cpp
  tools/cli/loadclientscliapp.cpp
  tools/cli/loadclientsconfigparser.cpp
  src/client/clientconfigvalidator.cpp
)
target_include_directories(will-load-clients PRIVATE
  src/client
  tools
  tools/cli
)
target_link_libraries(will-load-clients PRIVATE common CLI11::CLI11 Threads::Threads)

enable_testing()
add_subdirectory(tests)
