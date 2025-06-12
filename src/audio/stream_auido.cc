#include "auhub/audio/stream_auido.h"

namespace auhub {
namespace audio {

StreamAudio::StreamAudio(int sampleRate, int channels, size_t buffer_size)
    : buffer(buffer_size) {
  info.samplerate = sampleRate;
  info.channels = channels;
  assert(sampleRate == 16000 && channels == 1);
}

size_t StreamAudio::read(short *out_ptr, unsigned long n_samples) {
  const size_t read_samples = buffer.pop(out_ptr, n_samples);
  spdlog::debug("need {} frames; read {} frames", n_samples, read_samples);
  return read_samples;
}

void StreamAudio::push(const std::vector<short> &audio, bool complete) {
  const size_t pushed_samples = buffer.push(audio.data(), audio.size());

  if (pushed_samples < audio.size()) {
    spdlog::warn(
        "failed to push all samples (pushed={}, total={} buffer size={})",
        pushed_samples, audio.size(), buffer.read_available());
  }

  if (complete) {
    load_completed.store(true);
  }

  spdlog::debug("push {} samples, Audio load completed {}", pushed_samples,
                complete);
}

size_t StreamAudio::getRemainPCMCount() {
  return buffer.read_available();  // 直接返回队列中可读的数据量
}

}  // namespace audio
}  // namespace auhub