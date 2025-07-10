#include "auhub/player/uart_player.h"

#include "uart_common.h"

namespace auhub {
namespace player {
UartPlayer::UartPlayer() {}

bool UartPlayer::play_(audio::AudioBase *audio,
                       std::shared_ptr<PlayProgress> progress) {
  if (progress) {
    throw std::runtime_error("not support progress for uart player");
  }

  static bool inited = false;
  if (!inited) {
    Init_uart_thread(6);
    char device[] = "/dev/ttyACM0";
    spdlog::info("UartPlayer device:{}", device);
    startAudioThread(device);
    inited = true;
  }

  spdlog::info("audio play_uart_ playing...");

  // auto recv_warp = [this, audio]() mutable
  // {
  //     while (playing_.load() && audio)
  //     {
  //         int nRet = RecvAudioByUart(szRecvBuf);
  //         spdlog::info("RecvAudioByUart nRet:{}", nRet);
  //         (void)nRet; //? unused
  //     }
  // };
  // std::thread(recv_warp).detach();

  while (playing_.load() && audio) {
    if (audio->load_completed.load() && audio->getRemainPCMCount() <= 0) {
      spdlog::debug("UartPlayer completed");
      break;
    }

    int n_read =
        audio->read(reinterpret_cast<short *>(szSendBuf), BUFFER_SIZE / 2);
    if (n_read < BUFFER_SIZE / 2) {
      spdlog::warn("need to read {} frames, but only {}", BUFFER_SIZE / 2,
                   n_read);
    }

    //! 可能阻塞!!
    if (RecvAudioByUart(szRecvBuf)) {
      spdlog::debug("RecvAudioByUart success");
    } else {
      spdlog::warn("RecvAudioByUart failed");
    }
    sendAudioByUart(szSendBuf);

    usleep(10000);
  }
  return true;
}
}  // namespace player
}  // namespace auhub