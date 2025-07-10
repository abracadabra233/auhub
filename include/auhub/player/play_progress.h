#pragma once

#include <chrono>
#include <optional>
#include <semaphore>

class PlayProgress {
 private:
  float value_;
  std::binary_semaphore ready_{0};

 public:
  PlayProgress() = default;

  void Set(float value) {
    value_ = value;
    ready_.release();
  }

  float Get() {
    ready_.acquire();
    return value_;
  }

  float Get(const int timeout) {
    if (ready_.try_acquire_for(std::chrono::milliseconds(timeout))) {
      return value_;
    } else {
      return -1;
    }
  }
};
