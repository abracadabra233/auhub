#pragma once

#include "auhub/audio/base.h"

namespace auhub {
namespace audio {

class FileAudio : public AudioBase {
 private:
  SNDFILE *file = nullptr;

 public:
  explicit FileAudio(const std::string &filePath);
  ~FileAudio();

  size_t read(short *out_ptr, unsigned long n_frames) override;
  size_t getRemainPCMCount() override;
};

}  // namespace audio
}  // namespace auhub