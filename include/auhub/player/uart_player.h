#pragma once

#include "auhub/player/base.hpp"

namespace auhub {
namespace player {

const int SAMPLE_RATE = 16000;
const int BITS_PER_SAMPLE = 16;
const int CHANNELS = 1;
const int BUFFER_SIZE = 320;  // bytes
const int SAMPLES_PER_CHUNK = BUFFER_SIZE / (BITS_PER_SAMPLE / 8);

class UartPlayer : public PlayerBase<UartPlayer> {
  friend class PlayerBase<UartPlayer>;

 public:
  UartPlayer(const UartPlayer&) = delete;
  UartPlayer& operator=(const UartPlayer&) = delete;
  UartPlayer();

 protected:
  bool play_(audio::AudioBase* audio) override;

 private:
  char szSendBuf[BUFFER_SIZE];
  char szRecvBuf[BUFFER_SIZE];
};

}  // namespace player
}  // namespace auhub