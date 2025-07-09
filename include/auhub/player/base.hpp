#pragma once

#include <thread>

#include "auhub/audio/base.h"

namespace auhub {
namespace player {

template <typename T>
class PlayerBase {
 private:
  PlayerBase(const PlayerBase &) = delete;
  PlayerBase &operator=(const PlayerBase &) = delete;

  std::thread play_thread_;

 protected:
  PlayerBase() = default;
  ~PlayerBase() {
    if (play_thread_.joinable()) {
      play_thread_.join();
    }
  };

  virtual bool play_(audio::AudioBase *audio) = 0;
  static std::atomic<bool> playing_;

 public:
  // ? 这里没法搞成uniqptr，现代cpp单例就是std::shared_ptr
  template <typename... Args>
  static std::shared_ptr<PlayerBase> getInstance(Args &&...args) {
    static auto instance = std::make_shared<T>(std::forward<Args>(args)...);
    return instance;
  }

  void play(std::unique_ptr<audio::AudioBase> audio) {
    if (!audio) {
      spdlog::error("play audio failed: audio is null");
      return;
    }
    if (playing_.load()) {
      spdlog::warn("audio is playing,stop first!");
      stop();
    }
    if (play_thread_.joinable()) {
      play_thread_.join();
    }
    auto play_warp = [this, audio = std::move(audio)]() mutable {
      playing_.store(true);
      play_(audio.get());
      playing_.store(false);
    };
    play_thread_ = std::thread(std::move(play_warp));
  }
  void play(std::shared_ptr<audio::AudioBase> audio) {
    if (!audio) {
      spdlog::error("play audio failed: audio is null");
      return;
    }
    if (playing_.load()) {
      spdlog::warn("audio is playing,stop first!");
      stop();
    }
    if (play_thread_.joinable()) {
      play_thread_.join();
    }
    auto play_warp = [this, audio]() mutable {
      playing_.store(true);
      play_(audio.get());
      playing_.store(false);
    };
    play_thread_ = std::thread(play_warp);
  }
  void stop() {
    if (playing_.load()) {
      playing_.store(false);
    }
    if (play_thread_.joinable()) {
      play_thread_.join();
    }
  }
};

template <typename T>
std::atomic<bool> PlayerBase<T>::playing_ = false;

}  // namespace player
}  // namespace auhub