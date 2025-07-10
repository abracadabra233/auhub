#pragma once

#include <portaudio.h>

#include <memory>

#include "auhub/player/base.hpp"

namespace auhub {
namespace player {

class CardPlayer : public PlayerBase<CardPlayer> {
  friend class PlayerBase<CardPlayer>;

 public:
  CardPlayer(const CardPlayer &) = delete;
  CardPlayer &operator=(const CardPlayer &) = delete;

  CardPlayer();

 protected:
  bool play_(audio::AudioBase *audio,
             std::shared_ptr<PlayProgress> progress) override;

 private:
  static int paCallback(const void *, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags,
                        void *userData);
};

}  // namespace player
}  // namespace auhub