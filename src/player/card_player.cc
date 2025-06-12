#include "auhub/player/card_player.h"

#include <spdlog/spdlog.h>

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

bool CardPlayer::play_(audio::AudioBase *audio) {
  if (!audio || audio->info.channels <= 0) return false;

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
    Pa_Terminate();
    return false;
  }

  err = Pa_StartStream(pa_stream);
  if (err != paNoError) {
    spdlog::error("system error, failed to start stream:  {}",
                  Pa_GetErrorText(err));
    Pa_CloseStream(pa_stream);
    Pa_Terminate();
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
  static short buffer[128];

  auto *audio = static_cast<audio::AudioBase *>(userData);
  // todo 如果是多通道，得修改framesPerBuffer的值
  size_t readCount = audio->read(buffer, framesPerBuffer);

  float *out = static_cast<float *>(outputBuffer);
  std::transform(buffer, buffer + readCount, out, [](short sample) {
    return static_cast<float>(sample) / 32768.0f;
  });

  if (readCount < framesPerBuffer) {
    const sf_count_t remaining = framesPerBuffer - readCount;
    std::fill_n(out + readCount * audio->info.channels,
                remaining * audio->info.channels, 0.0f);

    if (audio->load_completed.load()) {
      spdlog::info("audio data already play finished");
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