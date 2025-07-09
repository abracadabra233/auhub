#include <cxxopts.hpp>
#include <variant>

#include "auhub/audio/file_audio.h"
#include "auhub/player/card_player.h"
#include "auhub/player/uart_player.h"

using namespace auhub::audio;
using namespace auhub::player;

inline auto parseOptions(int argc, char *argv[]) {
  cxxopts::Options options(argv[0], "Config");
  auto setting = options.add_options("opts");
  setting("f,filename", "audio file path", cxxopts::value<std::string>());
  setting("t,player_type", "player type",
          cxxopts::value<std::string>()->default_value("card"));
  setting("h,help", "Print usage");

  auto opts = options.parse(argc, argv);
  if (opts.count("help") || !opts.count("filename")) {
    spdlog::info(options.help());
    exit(1);
  }

  return opts;
}

using PlayerVariant = std::variant<std::shared_ptr<PlayerBase<UartPlayer>>,
                                   std::shared_ptr<PlayerBase<CardPlayer>>>;

void play_audio(const std::string &filename, const std::string &player_type) {
  auto audio = AudioBase::create<FileAudio>(filename);
  PlayerVariant player;

  if (player_type == "uart") {
    player = UartPlayer::getInstance();
  } else if (player_type == "card") {
    player = CardPlayer::getInstance();
  } else {
    throw std::invalid_argument(
        "Invalid player type. Must be 'uart' or 'card'");
  }

  // 使用std::visit访问variant
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

  auto filename = opts["filename"].as<std::string>();
  auto player_type = opts["player_type"].as<std::string>();

  play_audio(filename, player_type);
  return 0;
}