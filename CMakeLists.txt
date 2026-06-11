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
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Domain layer: stdlib only (no ASIO, SQLite, or wire protocol).
add_library(will-domain
  src/domain/usecases/send_chat_message.cpp
  src/domain/usecases/fetch_chat_history.cpp
  src/domain/usecases/authenticate_user.cpp
)
target_include_directories(will-domain PUBLIC src/domain)
target_compile_features(will-domain PUBLIC cxx_std_20)

# Wire codec (former src/common).
add_library(will-protocol
  src/infra/protocol/tcpframe.cpp
  src/infra/protocol/wiremessage_codec.cpp
  src/infra/protocol/wiremessage_codec_internal.cpp
  src/infra/protocol/wiremessage_messages.cpp
)
target_include_directories(will-protocol PUBLIC src/infra/protocol)
target_compile_features(will-protocol PUBLIC cxx_std_20)

add_executable(will-server
  src/server/server.cpp
  src/server/cli/servercliapp.cpp
  src/server/config/serverconfigparser.cpp
  src/server/tcpconnectionregistry.cpp
  src/server/config/serverconfigvalidator.cpp
  src/server/willserver.cpp
  src/server/asioserver.cpp
  src/server/iocontext.cpp
  src/server/iocontextthreadpool.cpp
  src/server/tcpconnection.cpp
  src/server/connectionaccountstore.cpp
  src/server/inbound_client_message_handler.cpp
  src/server/protocoladapter.cpp
  src/server/tcpconnectionparticipantnotifierimpl.cpp
  src/server/tcpframereader.cpp
  src/server/tcpframewriter.cpp
  src/infra/persistence/sqlite_database.cpp
  src/infra/persistence/password_hash.cpp
  src/infra/persistence/sqlite_message_repository_impl.cpp
  src/infra/persistence/sqlite_user_repository_impl.cpp
  src/infra/persistence/sqlite_auth_session_store_impl.cpp
  src/infra/persistence/sqlite_persistence_bundle.cpp
)
target_include_directories(will-server PRIVATE
  src/server
  src/server/cli
  src/server/config
  src/domain
  src/infra/persistence
  ${asio_SOURCE_DIR}/asio/include
)
target_compile_definitions(will-server PRIVATE ASIO_STANDALONE)
target_link_libraries(will-server PRIVATE will-domain will-protocol CLI11::CLI11 Threads::Threads
                                             SQLite::SQLite3 OpenSSL::Crypto)

add_executable(will-client
  src/client/client.cpp
  src/client/inbound_server_message_handler.cpp
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
target_link_libraries(will-client PRIVATE will-protocol CLI11::CLI11 Threads::Threads)

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
target_link_libraries(will-load-clients PRIVATE will-protocol CLI11::CLI11 Threads::Threads)

enable_testing()
add_subdirectory(tests)
