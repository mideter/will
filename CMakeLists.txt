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

find_package(Threads REQUIRED)

add_library(common
  src/common/clioption.cpp
  src/common/clicursor.cpp
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
  src/server/serverconfig.cpp
  src/server/cli/servercliparser.cpp
  src/server/cli/serverclioption.cpp
  src/server/sessionregistry.cpp
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
target_link_libraries(will-server PRIVATE common Threads::Threads)

add_executable(will-client
  src/client/client.cpp
  src/client/chatsession.cpp
  src/client/willclient.cpp
)
target_include_directories(will-client PRIVATE
  src/client
  ${asio_SOURCE_DIR}/asio/include
)
target_compile_definitions(will-client PRIVATE ASIO_STANDALONE)
target_link_libraries(will-client PRIVATE common Threads::Threads)

add_executable(will-load-clients
  tools/load_clients.cpp
)
target_include_directories(will-load-clients PRIVATE src/client src/common)
target_link_libraries(will-load-clients PRIVATE common Threads::Threads)

enable_testing()
add_executable(will-message-test tests/will_message_test.cpp)
target_link_libraries(will-message-test PRIVATE common)
add_test(NAME will-message COMMAND will-message-test)

add_executable(will-cli-test tests/cli_test.cpp
  src/server/serverconfig.cpp
  src/server/cli/serverclioption.cpp
)
target_include_directories(will-cli-test PRIVATE
  src/server
  src/server/cli
)
target_link_libraries(will-cli-test PRIVATE common)
add_test(NAME will-cli COMMAND will-cli-test)

add_executable(will-server-cli-integration-test tests/server_cli_integration_test.cpp)
add_dependencies(will-server-cli-integration-test will-server)
add_test(NAME will-server-cli-integration
  COMMAND will-server-cli-integration-test $<TARGET_FILE:will-server>)
