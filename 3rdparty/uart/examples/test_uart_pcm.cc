#include "uart_common.h"

#include <string>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

const int SAMPLE_RATE = 16000;
const int BITS_PER_SAMPLE = 16;
const int CHANNELS = 1;
const int BUFFER_SIZE = 320; // bytes
const int SAMPLES_PER_CHUNK = BUFFER_SIZE / (BITS_PER_SAMPLE / 8);

bool readPcmData(std::ifstream &pcmFile, char *buffer, size_t bufferSize)
{
    pcmFile.read(buffer, bufferSize);
    return pcmFile.gcount() > 0;
}

void processPcmFile(const char *filename)
{
    char device[] = "/dev/ttyACM0";
    Init_uart_thread(6);
    startAudioThread(device);

    char szSendBuf[BUFFER_SIZE];
    char szRecvBuf[BUFFER_SIZE]; // Assuming this exists for your UART communication

    std::ifstream pcmFile(filename, std::ios::binary);
    if (!pcmFile.is_open())
    {
        std::cerr << "Error opening PCM file" << std::endl;
        return;
    }

    while (true)
    {
        bool success = readPcmData(pcmFile, szSendBuf, sizeof(szSendBuf));
        if (!success)
        {
            break;
        }
        sendAudioByUart(szSendBuf);
        int nRet = RecvAudioByUart(szRecvBuf);

        usleep(10000);
    }

    pcmFile.close();
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <audiofile>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    processPcmFile(filename);
    return 0;
}
