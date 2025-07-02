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

size_t FileAudio::read(short *out_ptr, unsigned long n_frames) {
  if (!file || info.channels == 0) return 0; // Sanity check for channels
  // sf_read_short reads items (shorts). We want to read n_frames for all channels.
  size_t items_to_read = n_frames * info.channels;
  sf_count_t items_read = sf_read_short(file, out_ptr, items_to_read);

  // Return the number of frames read.
  return static_cast<size_t>(items_read / info.channels);
};

size_t FileAudio::getRemainPCMCount() {
  // This should return remaining frames. sf_seek with SEEK_CUR gives current frame position.
  // So, info.frames - current_frame_position is correct.
  return info.frames - sf_seek(file, 0, SEEK_CUR);
};
}  // namespace audio
}  // namespace auhub