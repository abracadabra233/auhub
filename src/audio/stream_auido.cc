#include "auhub/audio/stream_auido.h"

namespace auhub {
namespace audio {

StreamAudio::StreamAudio(int sampleRate, int channels) {
  info.samplerate = sampleRate;
  info.channels = channels;
  assert(sampleRate == 16000 && channels == 1);
}

size_t StreamAudio::read(short *out_ptr, unsigned long n_samples) {
  std::unique_lock<std::mutex> lock(buffer_mutex);
  if (n_samples > buffer.size()) {
    spdlog::warn("need {} frames, but only {} left in buffer", n_samples,
                 buffer.size());
    n_samples = buffer.size();
  }

  std::copy(buffer.begin(), buffer.begin() + n_samples, out_ptr);
  buffer.erase(buffer.begin(), buffer.begin() + n_samples);

  spdlog::info("read {} n_samples", n_samples);
  return n_samples;
}

void StreamAudio::push(const std::vector<short> &audio, bool complete) {
  std::lock_guard<std::mutex> lock(buffer_mutex);
  buffer.insert(buffer.end(), audio.begin(), audio.end());

  if (complete) {
    load_completed.store(true);
  }
  spdlog::info("push {}samples, load_completed {}", audio.size(), complete);
}

size_t StreamAudio::getRemainPCMCount() { return buffer.size(); };

}  // namespace audio
}  // namespace auhub