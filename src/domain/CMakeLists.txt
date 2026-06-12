# Domain layer: stdlib only (no ASIO, SQLite, or wire protocol).
add_library(will-domain
  support/phone_number.cpp
  usecases/send_chat_message.cpp
  usecases/fetch_chat_history.cpp
  usecases/authenticate_user.cpp
)
target_include_directories(will-domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(will-domain PUBLIC cxx_std_20)
