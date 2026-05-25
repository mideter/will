enable_testing()

add_executable(will-message-test will_message_test.cpp)
target_link_libraries(will-message-test PRIVATE common)
add_test(NAME will-message COMMAND will-message-test)

add_executable(will-server-config-test server_config_test.cpp
  ${CMAKE_SOURCE_DIR}/src/server/serverconfigvalidator.cpp
)
target_include_directories(will-server-config-test PRIVATE
  ${CMAKE_SOURCE_DIR}/src/server
)
add_test(NAME will-server-config COMMAND will-server-config-test)

add_executable(will-cli-test cli_test.cpp
  ${CMAKE_SOURCE_DIR}/src/server/cli/servercliapp.cpp
)
target_include_directories(will-cli-test PRIVATE
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/server/cli
)
target_link_libraries(will-cli-test PRIVATE common CLI11::CLI11)
add_test(NAME will-cli COMMAND will-cli-test)

add_executable(will-client-cli-test client_cli_test.cpp
  ${CMAKE_SOURCE_DIR}/src/client/clientconfig.cpp
  ${CMAKE_SOURCE_DIR}/src/client/cli/clientcliapp.cpp
)
target_include_directories(will-client-cli-test PRIVATE
  ${CMAKE_SOURCE_DIR}/src/client
  ${CMAKE_SOURCE_DIR}/src/client/cli
)
target_link_libraries(will-client-cli-test PRIVATE common CLI11::CLI11)
add_test(NAME will-client-cli COMMAND will-client-cli-test)

add_executable(will-server-cli-integration-test server_cli_integration_test.cpp)
target_link_libraries(will-server-cli-integration-test PRIVATE CLI11::CLI11)
add_dependencies(will-server-cli-integration-test will-server)
add_test(NAME will-server-cli-integration
  COMMAND will-server-cli-integration-test $<TARGET_FILE:will-server>)
