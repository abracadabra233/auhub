#pragma once

#include <portaudio.h>

#include <memory>

#include "auhub/player/base.hpp"
#include "auhub/player/blocking_value.hpp"

namespace auhub {
namespace player {

class CardPlayer : public PlayerBase<CardPlayer> {
  friend class PlayerBase<CardPlayer>;

 public:
  CardPlayer(const CardPlayer &) = delete;
  CardPlayer &operator=(const CardPlayer &) = delete;

  CardPlayer(std::shared_ptr<BlockingValue<float>> progress = nullptr);
  ~CardPlayer();

 protected:
  bool play_(audio::AudioBase *audio) override;

 private:
  static int paCallback(const void *, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags,
                        void *userData);

  static std::shared_ptr<BlockingValue<float>> progress_;
};

}  // namespace player
}  // namespace auhub