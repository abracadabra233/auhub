#include "uart_common.h"

#include <string>
#include <sndfile.h>
#include <iostream>
#include <unistd.h>
#include <filesystem>
namespace fs = std::filesystem;

const int BUFFER_SIZE = 320; // bytes

void run(const std::string &filePath)
{
    SF_INFO info{};
    SNDFILE *file = sf_open(filePath.c_str(), SFM_READ, &info);
    if (!file)
    {
        // 处理文件打开失败的情况
        std::cerr << "Error opening audio file: " << sf_strerror(nullptr) << std::endl;
        return;
    }
    std::cout << "Audio file opened successfully! sr:" << info.samplerate << std::endl;

    char device[] = "/dev/ttyACM0";
    Init_uart_thread(6);
    startAudioThread(device);

    // 确保音频格式是16位PCM
    if (info.format != (SF_FORMAT_WAV | SF_FORMAT_PCM_16) &&
        info.format != (SF_FORMAT_RAW | SF_FORMAT_PCM_16))
    {
        std::cerr << "Audio format is not 16-bit PCM" << std::endl;
        sf_close(file);
        return;
    }
    char szRecvBuf[BUFFER_SIZE];
    char szSendBuf[BUFFER_SIZE];

    while (true)
    {
        sf_count_t framesRead = sf_readf_short(file, reinterpret_cast<short *>(szSendBuf), BUFFER_SIZE / 2);
        if (framesRead <= 0)
        {
            break;
        }

        sendAudioByUart(szSendBuf);
        RecvAudioByUart(szRecvBuf);
        usleep(10000);
    }
    sf_close(file);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <audiofile>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    run(filename);
    return 0;
}
