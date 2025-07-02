#include <cxxopts.hpp>

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

void play_audio(const std::string &filename, const std::string &player_type) {
  std::unique_ptr<PlayerBase> player = nullptr;
  if (player_type == "uart") {
    player = std::make_unique<UartPlayer>();
  } else if (player_type == "card") {
    player = std::make_unique<CardPlayer>();
  } else {
    throw std::invalid_argument(
        "Invalid player type. Must be 'uart' or 'card'");
  }

  std::unique_ptr<AudioBase> audio_source = AudioBase::create<FileAudio>(filename);

  if (!audio_source) {
    spdlog::error("Failed to create audio source for file: {}", filename);
    return;
  }

  // Log audio properties
  spdlog::info("Successfully loaded audio file: {}", filename);
  spdlog::info("  Sample Rate: {} Hz", audio_source->info.samplerate);
  spdlog::info("  Channels: {}", audio_source->info.channels);
  spdlog::info("  Frames: {}", audio_source->info.frames);
  spdlog::info("  Format (libsndfile): {:#x}", audio_source->info.format); // Print format as hex

  // Determine bit depth from sf_info.format (simplified)
  std::string bit_depth_str = "Unknown";
  int main_format = audio_source->info.format & SF_FORMAT_TYPEMASK;
  int sub_format = audio_source->info.format & SF_FORMAT_SUBMASK;

  if (main_format == SF_FORMAT_PCM_S8 || main_format == SF_FORMAT_PCM_U8) bit_depth_str = "8-bit";
  else if (sub_format == SF_FORMAT_PCM_16) bit_depth_str = "16-bit";
  else if (sub_format == SF_FORMAT_PCM_24) bit_depth_str = "24-bit";
  else if (sub_format == SF_FORMAT_PCM_32) bit_depth_str = "32-bit";
  else if (sub_format == SF_FORMAT_FLOAT) bit_depth_str = "32-bit float";
  else if (sub_format == SF_FORMAT_DOUBLE) bit_depth_str = "64-bit double";
  else if (main_format == SF_FORMAT_FLAC && sub_format == 0) bit_depth_str = "FLAC (variable, decoded to 16-bit by sf_read_short)"; // FLAC default
  else if (main_format == SF_FORMAT_VORBIS && sub_format == 0) bit_depth_str = "Vorbis (decoded to 16-bit by sf_read_short)";
  // Add more for MP3 etc. if libsndfile provides specific format codes after opening
  // For many compressed formats, libsndfile converts to PCM, and sf_read_short gives 16-bit.
  // The SF_INFO.format reflects the original file's format.

  spdlog::info("  Reported Bit Depth (approx): {}", bit_depth_str);


  player->play(std::move(audio_source));

  // Keep playing until player stops or a timeout (e.g. 60 seconds for longer files)
  // PlayerBase::isPlaying() is used to check status.
  int max_play_duration_seconds = 60;
  for (int i = 0; i < max_play_duration_seconds * 10; ++i) { // Check every 100ms
      if (!player->isPlaying()) {
          break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (player->isPlaying()) {
      spdlog::info("Playback timeout reached, stopping player.");
      player->stop();
  } else {
      spdlog::info("Playback finished or was stopped.");
  }
}

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);

  auto opts = parseOptions(argc, argv);

  auto filename = opts["filename"].as<std::string>();
  auto player_type = opts["player_type"].as<std::string>();

  play_audio(filename, player_type);
  return 0;
}