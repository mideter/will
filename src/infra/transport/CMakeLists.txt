add_library(will-transport
  tcpframedchannel.cpp
  tcpframedchannel_state.cpp
  tcpframereader.cpp
  tcpframewriter.cpp
)
target_include_directories(will-transport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                                 ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(will-transport PUBLIC ASIO_STANDALONE)
target_compile_features(will-transport PUBLIC cxx_std_20)
target_link_libraries(will-transport PUBLIC will-protocol Threads::Threads)
