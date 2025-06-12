#pragma once

#include <boost/lockfree/spsc_queue.hpp>

#include "auhub/audio/base.h"

namespace auhub {
namespace audio {

class StreamAudio : public AudioBase {
 private:
  boost::lockfree::spsc_queue<short> buffer;

 public:
  StreamAudio(int sampleRate, int channels, size_t buffer_size = 1024 * 1024);
  void push(const std::vector<short> &audio, bool complete);

  size_t read(short *out_ptr, unsigned long n_samples) override;
  size_t getRemainPCMCount() override;
};
}  // namespace audio
}  // namespace auhub