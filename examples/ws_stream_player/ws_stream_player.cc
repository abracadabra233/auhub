#include <cxxopts.hpp>
#include <variant>

#include "auhub/player/card_player.h"
#include "auhub/player/uart_player.h"
#include "ws_stream_auido.h"

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

using PlayerVariant = std::variant<std::shared_ptr<PlayerBase<UartPlayer>>,
                                   std::shared_ptr<PlayerBase<CardPlayer>>>;

void play_audio(const int ws_port, const std::string &player_type) {
  auto audio = AudioBase::create<WsStreamAudio>(ws_port, 16000, 1);
  PlayerVariant player;

  if (player_type == "uart") {
    player = UartPlayer::getInstance();
  } else if (player_type == "card") {
    player = CardPlayer::getInstance();
  } else {
    throw std::invalid_argument(
        "Invalid player type. Must be 'uart' or 'card'");
  }

  std::visit(
      [&audio](auto &&p) {
        p->play(std::move(audio));
        std::this_thread::sleep_for(std::chrono::seconds(5));
        p->stop();
      },
      player);
}

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);

  auto opts = parseOptions(argc, argv);

  auto ws_port = opts["port"].as<int>();
  auto player_type = opts["player_type"].as<std::string>();

  play_audio(ws_port, player_type);
  return 0;
}