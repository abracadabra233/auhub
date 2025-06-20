#include "auhub/player/base.h"

#include <spdlog/spdlog.h>

namespace auhub {
namespace player {

std::atomic<bool> PlayerBase::playing_{false};

void PlayerBase::play(std::unique_ptr<audio::AudioBase> audio) {
  if (!audio) {
    spdlog::error("play audio failed: audio is null");
    return;
  }
  if (playing_.load()) {
    spdlog::warn("audio is playing,stop first!");
    stop();
  }
  // todo?按理应该不需要这段，play_thread_执行完后应该自动销毁，第二次播放时不需要手动join
  if (play_thread_.joinable()) {
    play_thread_.join();
    spdlog::warn("audio play thread is joinable,join it!");
  }

  auto play_warp = [this, audio = std::move(audio)]() mutable {
    spdlog::info("audio play started");
    playing_.store(true);
    play_(audio.get());
    playing_.store(false);
    spdlog::info("audio play finished");
  };
  play_thread_ = std::thread(std::move(play_warp));
  spdlog::info("start audio play thread");
}

void PlayerBase::play(std::shared_ptr<audio::AudioBase> audio) {
  if (!audio) {
    spdlog::error("play audio failed: audio is null");
    return;
  }
  if (playing_.load()) {
    spdlog::warn("audio is playing,stop first!");
    stop();
  }
  // todo?按理应该不需要这段，play_thread_执行完后应该自动销毁，第二次播放时不需要手动join
  if (play_thread_.joinable()) {
    play_thread_.join();
    spdlog::warn("audio play thread is joinable,join it!");
  }

  auto play_warp = [this, audio]() mutable {
    spdlog::info("audio play started");
    playing_.store(true);
    play_(audio.get());
    playing_.store(false);
    spdlog::info("audio play finished");
  };
  play_thread_ = std::thread(play_warp);
  spdlog::info("start audio play thread");
}

void PlayerBase::stop() {
  if (playing_.load()) {
    playing_.store(false);
    spdlog::info("audio play stopped: playing_ set false");
  }
  if (play_thread_.joinable()) {
    play_thread_.join();
    spdlog::info("audio play stopped: play_thread_ join");
  }
}

bool PlayerBase::isPlaying() const { return playing_.load(); }

PlayerBase::~PlayerBase() {
  if (play_thread_.joinable()) {
    play_thread_.join();
  }
}
}  // namespace player
}  // namespace auhub