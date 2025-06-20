find_package(cxxopts CONFIG REQUIRED)

add_executable(ws_stream_player ws_stream_player.cc ws_stream_auido.cc)
target_link_libraries(ws_stream_player
    cxxopts::cxxopts
    auhub)
