#pragma once

#include <deque>
#include <vector> // For std::vector in push()
#include <mutex>  // For std::mutex

#include "auhub/audio/base.h"

namespace auhub {
namespace audio {

class StreamAudio : public AudioBase {
 private:
  std::deque<short> buffer;  // 音频数据缓冲队列
  mutable std::mutex buffer_mutex;   // 保护缓冲队列的互斥锁, mutable for const methods

 public:
  StreamAudio(int sampleRate, int channels);
  void push(const std::vector<short> &audio, bool complete);
  size_t read(short *out_ptr, unsigned long n_frames) override;
  size_t getRemainPCMCount() const override; // Made const
};
}  // namespace audio
}  // namespace auhub