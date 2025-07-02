#include "auhub/player/card_player.h"

#include <spdlog/spdlog.h>
#include <vector> // For std::vector

namespace auhub {
namespace player {

CardPlayer::CardPlayer() {
  PaError err = Pa_Initialize();
  if (err != paNoError) {
    spdlog::error("system error, portAudio init failed:  {}",
                  Pa_GetErrorText(err));
    throw std::runtime_error("portAudio init failed");
  }
}

CardPlayer::~CardPlayer() {
  PaError err = Pa_Terminate();
  if (err != paNoError) {
    spdlog::error("system error, portAudio terminate failed: {}",
                  Pa_GetErrorText(err));
  }
}

bool CardPlayer::play_(audio::AudioBase *audio) {
  if (!audio || audio->info.channels <= 0) {
    spdlog::error("CardPlayer::play_ called with invalid audio source.");
    return false;
  }
  if (audio->info.samplerate <= 0) {
    spdlog::error("CardPlayer::play_ audio source has invalid sample rate: {}", audio->info.samplerate);
    return false;
  }


  PaStreamParameters params;
  params.device = Pa_GetDefaultOutputDevice();
  params.channelCount = audio->info.channels;
  params.sampleFormat = paFloat32;
  params.suggestedLatency =
      Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->defaultLowOutputLatency;
  params.hostApiSpecificStreamInfo = nullptr;
  PaStream *pa_stream;
  PaError err;

  err =
      Pa_OpenStream(&pa_stream, nullptr, &params, audio->info.samplerate,
                    paFramesPerBufferUnspecified, paClipOff, paCallback, audio);
  if (err != paNoError) {
    spdlog::error("system error, failed to open stream:  {}",
                  Pa_GetErrorText(err));
    return false;
  }

  err = Pa_StartStream(pa_stream);
  if (err != paNoError) {
    spdlog::error("system error, failed to start stream:  {}",
                  Pa_GetErrorText(err));
    Pa_CloseStream(pa_stream); // Close the stream if start fails
    return false;
  }

  spdlog::info("audio playing...");
  while (Pa_IsStreamActive(pa_stream)) {
    Pa_Sleep(100);
  }

  err = Pa_StopStream(pa_stream);
  Pa_CloseStream(pa_stream);
  return err == paNoError;
}

int CardPlayer::paCallback(const void *, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo *,
                           PaStreamCallbackFlags, void *userData) {
  if (!playing_.load()) {
    return paComplete;
  }

  auto *audio = static_cast<audio::AudioBase *>(userData);
  if (!audio || audio->info.channels == 0) { // Basic check
    return paAbort; // Or paComplete, depending on desired behavior for bad state
  }

  // Dynamically allocate buffer for interleaved short samples
  std::vector<short> local_buffer(framesPerBuffer * audio->info.channels);

  // audio->read expects number of frames, and reads frames * channels samples
  size_t frames_read = audio->read(local_buffer.data(), framesPerBuffer);

  float *out = static_cast<float *>(outputBuffer);
  size_t samples_read = frames_read * audio->info.channels;

  // Convert read samples (shorts) to float output
  std::transform(local_buffer.data(), local_buffer.data() + samples_read, out, [](short sample) {
    return static_cast<float>(sample) / 32768.0f; // Assuming 16-bit signed PCM
  });

  // If fewer frames were read than requested by PortAudio, fill the rest with silence.
  if (frames_read < framesPerBuffer) {
    size_t remaining_frames = framesPerBuffer - frames_read;
    size_t remaining_samples = remaining_frames * audio->info.channels;
    float *fill_start_ptr = out + samples_read; // Start filling after the data we just wrote

    std::fill_n(fill_start_ptr, remaining_samples, 0.0f);

    if (audio->load_completed.load()) {
      // This condition might be true if it's the very end of the stream
      // and load_completed was set by another thread right after audio->read returned less.
      // Or if audio->read itself indicates end-of-stream and sets load_completed.
      // For now, we trust that if frames_read < framesPerBuffer, it's either EOF or an issue.
      spdlog::info("Audio data play finished or stream ended prematurely.");
      return paComplete;
    } else {
      spdlog::warn("need to read {} frames, but audio remain_framse {}",
                   framesPerBuffer, readCount);
      return paContinue;
    }
  }
  return paContinue;
}

}  // namespace player
}  // namespace auhub