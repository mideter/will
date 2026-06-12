# Wire codec (former src/common).
add_library(will-protocol
  tcpframe.cpp
  wiremessage_codec.cpp
  wiremessage_codec_internal.cpp
  wiremessage_messages.cpp
)
target_include_directories(will-protocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(will-protocol PRIVATE ${CMAKE_SOURCE_DIR}/src/domain)
target_compile_features(will-protocol PUBLIC cxx_std_20)
target_link_libraries(will-protocol PRIVATE will-domain)
