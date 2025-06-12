#include "auhub/audio/file_audio.h"

namespace auhub {
namespace audio {

FileAudio::FileAudio(const std::string &filePath) {
  file = sf_open(filePath.c_str(), SFM_READ, &info);
  if (!file) {
    throw std::runtime_error(filePath + " is not exist");
  }
  load_completed.store(true);
};

FileAudio::~FileAudio() {
  if (file) {
    sf_close(file);
  }
};

size_t FileAudio::read(short *out_ptr, unsigned long n_samples) {
  if (!file) return 0;
  size_t readSize = sf_read_short(file, out_ptr, n_samples);

  return readSize;
};

size_t FileAudio::getRemainPCMCount() {
  return info.frames - sf_seek(file, 0, SEEK_CUR);
};
}  // namespace audio
}  // namespace auhub