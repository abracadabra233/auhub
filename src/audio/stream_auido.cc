#include "auhub/audio/stream_auido.h"

namespace auhub {
namespace audio {

StreamAudio::StreamAudio(int sampleRate, int channels) {
  info.samplerate = sampleRate;
  info.channels = channels;
  // Removed: assert(sampleRate == 16000 && channels == 1);
  // It's also good practice to set other relevant SF_INFO fields if known,
  // e.g., info.format could be set to SF_FORMAT_PCM_16 if the stream is always 16-bit.
  // For now, samplerate and channels are the primary concern for this refactor.
  info.frames = 0; // Stream initially has 0 frames; this might not be strictly used by player for streams
  info.format = SF_FORMAT_PCM_16; // Assuming pushed data is 16-bit PCM
}

size_t StreamAudio::read(short *out_ptr, unsigned long n_frames) {
  if (info.channels == 0) return 0; // Avoid division by zero if not properly initialized

  std::unique_lock<std::mutex> lock(buffer_mutex);

  unsigned long samples_to_read = n_frames * info.channels;
  unsigned long samples_available = buffer.size();

  if (samples_to_read > samples_available) {
    spdlog::warn("Requested {} frames ({} samples), but only {} samples available in buffer",
                 n_frames, samples_to_read, samples_available);
    samples_to_read = samples_available;
  }

  if (samples_to_read == 0) {
    return 0; // No data to read or requested 0 frames
  }

  std::copy(buffer.begin(), buffer.begin() + samples_to_read, out_ptr);
  buffer.erase(buffer.begin(), buffer.begin() + samples_to_read);

  size_t frames_read = samples_to_read / info.channels;
  spdlog::debug("StreamAudio::read: req {} frames, read {} frames ({} samples)", n_frames, frames_read, samples_to_read);
  return frames_read;
}

void StreamAudio::push(const std::vector<short> &audio, bool complete) {
  std::lock_guard<std::mutex> lock(buffer_mutex);
  buffer.insert(buffer.end(), audio.begin(), audio.end());

  if (complete) {
    load_completed.store(true);
  }
  spdlog::debug("StreamAudio::push: pushed {} samples, complete: {}", audio.size(), complete);
}

size_t StreamAudio::getRemainPCMCount() const {
  if (info.channels == 0) return 0; // Avoid division by zero
  return buffer.size() / info.channels; // Returns remaining frames
};

}  // namespace audio
}  // namespace auhub