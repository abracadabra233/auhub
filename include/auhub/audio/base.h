#pragma once

#include <sndfile.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <memory>

namespace auhub {
namespace audio {

class AudioBase {
 public:
  SF_INFO info{};
  std::atomic<bool> load_completed{false};
  // Read 'n_frames' of audio data into out_ptr.
  // For stereo audio, this means n_frames * 2 short samples will be read.
  // Returns the number of frames actually read.
  virtual size_t read(short *out_ptr, unsigned long n_frames) = 0;
  virtual size_t getRemainPCMCount() = 0;

  ~AudioBase() = default;
  AudioBase() = default;

  template <typename T, typename... Args>
  static std::unique_ptr<AudioBase> create(Args &&...args) {
    try {
      return std::make_unique<T>(std::forward<Args>(args)...);
    } catch (const std::exception &e) {
      spdlog::error("Failed to create AudioBase: {}", e.what());
      return nullptr;
    }
  }
};

}  // namespace audio
}  // namespace auhub