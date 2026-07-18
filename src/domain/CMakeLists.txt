# Domain layer: stdlib only (no ASIO, SQLite, or wire protocol).
add_library(will-domain
  support/device_token.cpp
  support/user_name.cpp
  usecases/send_chat_message.cpp
  usecases/fetch_chat_history.cpp
  usecases/authenticate_device.cpp
)
target_include_directories(will-domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(will-domain PUBLIC cxx_std_20)
