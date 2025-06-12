#pragma once

#include <portaudio.h>

#include "auhub/player/base.h"

namespace auhub {
namespace player {

class CardPlayer : public PlayerBase {
 public:
  CardPlayer();

 protected:
  bool play_(audio::AudioBase *audio) override;

 private:
  static int paCallback(const void *, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags,
                        void *userData);
};

}  // namespace player
}  // namespace auhub