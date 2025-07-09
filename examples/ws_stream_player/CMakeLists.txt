find_package(cxxopts CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio beast)

add_executable(ws_stream_player ws_stream_player.cc ws_stream_auido.cc)
target_link_libraries(ws_stream_player
    Boost::asio
    Boost::beast
    cxxopts::cxxopts
    auhub)
