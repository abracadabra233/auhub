#include "auhub/player/card_player.h"

#include <spdlog/spdlog.h>

#include <algorithm>

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

struct CallbackData {
  audio::AudioBase *audio;
  std::shared_ptr<PlayProgress> progress;
};

bool CardPlayer::play_(audio::AudioBase *audio,
                       std::shared_ptr<PlayProgress> progress) {
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

  auto callbackData =
      std::make_unique<CallbackData>(CallbackData{audio, progress});

  err = Pa_OpenStream(&pa_stream, nullptr, &params, audio->info.samplerate,
                      paFramesPerBufferUnspecified, paClipOff, paCallback,
                      callbackData.get());
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

  auto *data = static_cast<CallbackData *>(userData);
  auto *audio = data->audio;
  auto progress_ptr = data->progress;

  // todo 如果是多通道，得修改framesPerBuffer的值
  size_t readCount = audio->read(buffer, framesPerBuffer);

  float *out = static_cast<float *>(outputBuffer);
  std::transform(buffer, buffer + readCount, out, [](short sample) {
    return static_cast<float>(sample) / 32768.0f;
  });

  if (progress_ptr) {
    float progress = 1.0f - (static_cast<float>(audio->getRemainPCMCount()) /
                             audio->info.frames);
    progress_ptr->Set(progress);
  }
  if (readCount < framesPerBuffer) {
    const sf_count_t remaining = framesPerBuffer - readCount;
    std::fill_n(out + readCount * audio->info.channels,
                remaining * audio->info.channels, 0.0f);

    if (audio->load_completed.load()) {
      spdlog::debug("audio data already play finished");
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