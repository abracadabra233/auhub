#pragma once

#include <thread>

#include "auhub/audio/base.h"

namespace auhub {
namespace player {

class PlayerBase {
 private:
  PlayerBase(const PlayerBase &) = delete;
  PlayerBase &operator=(const PlayerBase &) = delete;

  std::thread play_thread_;

 protected:
  virtual bool play_(audio::AudioBase *audio) = 0;
  static std::atomic<bool> playing_;

 public:
  ~PlayerBase();
  PlayerBase() = default;

  void play(std::unique_ptr<audio::AudioBase> audio);
  void play(std::shared_ptr<audio::AudioBase> audio);
  void stop();
  bool isPlaying() const;
};
}  // namespace player
}  // namespace auhub