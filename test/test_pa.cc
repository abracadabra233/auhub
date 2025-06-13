#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#include "portaudio.h"

// WAV文件头结构
struct WavHeader {
  char chunkId[4];
  uint32_t chunkSize;
  char format[4];
  char subchunk1Id[4];
  uint32_t subchunk1Size;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
  char subchunk2Id[4];
  uint32_t subchunk2Size;
};

int main() {
  // 初始化PortAudio
  PaError err = Pa_Initialize();
  if (err != paNoError) {
    std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
    return -1;
  }

  // 打开WAV文件
  std::ifstream file(
      "/home/wx/code/loong/loong_exp/cpp_exp/audio_player/examples/sources/"
      "test.wav",
      std::ios::binary);
  if (!file) {
    std::cerr << "Failed to open WAV file" << std::endl;
    Pa_Terminate();
    return -1;
  }

  // 读取WAV头
  WavHeader header;
  file.read(reinterpret_cast<char*>(&header), sizeof(header));

  // 读取音频数据
  std::vector<char> data(header.subchunk2Size);
  file.read(data.data(), header.subchunk2Size);
  file.close();

  // 打开默认输出设备
  PaStream* stream;
  err = Pa_OpenDefaultStream(&stream,
                             0,                   // 输入通道数
                             header.numChannels,  // 输出通道数
                             paInt16,             // 采样格式
                             header.sampleRate,   // 采样率
                             256,                 // 缓冲区大小
                             nullptr,  // 回调函数（此处使用阻塞模式）
                             nullptr   // 用户数据
  );

  // 播放音频
  err = Pa_StartStream(stream);
  err = Pa_WriteStream(stream, data.data(), header.subchunk2Size / 2);
  // err = Pa_StopStream(stream);
  // err = Pa_CloseStream(stream);

  Pa_Terminate();
  std::cout << "Playback complete" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(10));
  return 0;
}
