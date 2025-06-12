#include <cxxopts.hpp>

#include "auhub/audio/ws_stream_auido.h"
#include "auhub/player/card_player.h"
#include "auhub/player/uart_player.h"

using namespace auhub::audio;
using namespace auhub::player;

inline auto parseOptions(int argc, char *argv[]) {
  cxxopts::Options options(argv[0], "Config");
  auto setting = options.add_options("opts");
  setting("p,port", "ws audio port",
          cxxopts::value<int>()->default_value("8081"));
  setting("t,player_type", "player type",
          cxxopts::value<std::string>()->default_value("card"));
  setting("h,help", "Print usage");

  auto opts = options.parse(argc, argv);
  if (opts.count("help")) {
    spdlog::info(options.help());
    exit(1);
  }

  return opts;
}

void play_audio(const int ws_port, const std::string &player_type) {
  std::unique_ptr<PlayerBase> player = nullptr;
  if (player_type == "uart") {
    player = std::make_unique<UartPlayer>();
    spdlog::info("uart player {}", ws_port);
  } else if (player_type == "card") {
    player = std::make_unique<CardPlayer>();
    spdlog::info("card player {}", ws_port);
  } else {
    throw std::invalid_argument(
        "Invalid player type. Must be 'uart' or 'card'");
  }

  std::unique_ptr<AudioBase> audio =
      AudioBase::create<WsStreamAudio>(ws_port, 16000, 1);

  player->play(std::move(audio));
  std::this_thread::sleep_for(std::chrono::seconds(500));
}

int main(int argc, char *argv[]) {
  auto opts = parseOptions(argc, argv);

  auto ws_port = opts["port"].as<int>();
  auto player_type = opts["player_type"].as<std::string>();

  play_audio(ws_port, player_type);
  return 0;
}