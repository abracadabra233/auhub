#pragma once

#include <deque>

#include "auhub/audio/base.h"

namespace auhub {
namespace audio {

class StreamAudio : public AudioBase {
 private:
  std::deque<short> buffer;  // 音频数据缓冲队列
  std::mutex buffer_mutex;   // 保护缓冲队列的互斥锁

 public:
  StreamAudio(int sampleRate, int channels);
  void push(const std::vector<short> &audio, bool complete);
  size_t read(short *out_ptr, unsigned long n_samples) override;
  size_t getRemainPCMCount() override;
};
}  // namespace audio
}  // namespace auhub