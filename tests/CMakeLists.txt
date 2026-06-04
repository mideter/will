enable_testing()

add_executable(will-message-test will_message_test.cpp)
target_link_libraries(will-message-test PRIVATE will-protocol)
add_test(NAME will-message COMMAND will-message-test)

add_executable(will-server-config-test server_config_test.cpp
  ${CMAKE_SOURCE_DIR}/src/server/config/serverconfigvalidator.cpp
)
target_include_directories(will-server-config-test PRIVATE
  ${CMAKE_SOURCE_DIR}/src/server/config
)
add_test(NAME will-server-config COMMAND will-server-config-test)

add_executable(will-cli-test cli_test.cpp
  ${CMAKE_SOURCE_DIR}/src/server/cli/servercliapp.cpp
)
target_include_directories(will-cli-test PRIVATE
  ${CMAKE_SOURCE_DIR}/src/server/config
  ${CMAKE_SOURCE_DIR}/src/server/cli
)
target_link_libraries(will-cli-test PRIVATE will-protocol CLI11::CLI11)
add_test(NAME will-cli COMMAND will-cli-test)

add_executable(will-client-config-test client_config_test.cpp
  ${CMAKE_SOURCE_DIR}/src/client/clientconfigvalidator.cpp
)
target_include_directories(will-client-config-test PRIVATE
  ${CMAKE_SOURCE_DIR}/src/client
  ${asio_SOURCE_DIR}/asio/include
)
target_compile_definitions(will-client-config-test PRIVATE ASIO_STANDALONE)
add_test(NAME will-client-config COMMAND will-client-config-test)

add_executable(will-client-cli-test client_cli_test.cpp
  ${CMAKE_SOURCE_DIR}/src/client/cli/clientcliapp.cpp
)
target_include_directories(will-client-cli-test PRIVATE
  ${CMAKE_SOURCE_DIR}/src/client
  ${CMAKE_SOURCE_DIR}/src/client/cli
)
target_link_libraries(will-client-cli-test PRIVATE will-protocol CLI11::CLI11)
add_test(NAME will-client-cli COMMAND will-client-cli-test)

add_executable(will-client-cli-integration-test client_cli_integration_test.cpp)
target_link_libraries(will-client-cli-integration-test PRIVATE CLI11::CLI11)
add_dependencies(will-client-cli-integration-test will-client)
add_test(NAME will-client-cli-integration
  COMMAND will-client-cli-integration-test $<TARGET_FILE:will-client>)

add_executable(will-server-cli-integration-test server_cli_integration_test.cpp)
target_link_libraries(will-server-cli-integration-test PRIVATE CLI11::CLI11)
add_dependencies(will-server-cli-integration-test will-server)
add_test(NAME will-server-cli-integration
  COMMAND will-server-cli-integration-test $<TARGET_FILE:will-server>)

add_executable(will-history-integration-test history_integration_test.cpp)
target_link_libraries(will-history-integration-test PRIVATE will-protocol)
add_dependencies(will-history-integration-test will-server)
add_test(NAME will-history-integration
  COMMAND will-history-integration-test $<TARGET_FILE:will-server>)

add_executable(will-sqlite-persistence-test sqlite_persistence_test.cpp
  ${CMAKE_SOURCE_DIR}/src/infra/persistence/sqlite_database.cpp
  ${CMAKE_SOURCE_DIR}/src/infra/persistence/password_hash.cpp
  ${CMAKE_SOURCE_DIR}/src/infra/persistence/sqlite_message_repository_impl.cpp
  ${CMAKE_SOURCE_DIR}/src/infra/persistence/sqlite_user_repository_impl.cpp
  ${CMAKE_SOURCE_DIR}/src/infra/persistence/sqlite_auth_session_store_impl.cpp
)
target_include_directories(will-sqlite-persistence-test PRIVATE
  ${CMAKE_SOURCE_DIR}/src/infra/persistence
  ${CMAKE_SOURCE_DIR}/src/domain
)
target_link_libraries(will-sqlite-persistence-test PRIVATE will-domain SQLite::SQLite3
                                                       OpenSSL::Crypto)
add_test(NAME will-sqlite-persistence COMMAND will-sqlite-persistence-test)
