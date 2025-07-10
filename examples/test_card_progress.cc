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
  setting("h,help", "Print usage");

  auto opts = options.parse(argc, argv);
  if (opts.count("help") || !opts.count("filename")) {
    spdlog::info(options.help());
    exit(1);
  }

  return opts;
}

void play_audio(const std::string &filename) {
  auto audio = AudioBase::create<FileAudio>(filename);
  auto player = CardPlayer::getInstance();

  std::shared_ptr<PlayProgress> progress = std::make_shared<PlayProgress>();
  player->play(std::move(audio), progress);

  while (true) {
    float p = progress->Get();
    spdlog::info("progress: {}", p);
    if (p >= 1.0) {
      spdlog::info("play finished");
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);

  auto opts = parseOptions(argc, argv);

  auto filename = opts["filename"].as<std::string>();
  play_audio(filename);
  return 0;
}