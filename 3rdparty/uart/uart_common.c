/*
 * @Author: zmz zhangmingzhi424@qq.com
 * @Date: 2024-04-12 09:49:10
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h> // Strlen function
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <unistd.h>
#include <linux/types.h>

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>

#include "fcntl.h"

#include "ringbuf.h"

// ARM_GCC=1   Ubuntu_GCC=2
#if Complie_Type == 2
#define LOCAL_SERVER
#endif

bool rs422Debug = true;

static int audioUartFd = 0;
static pthread_t tid_rs422;
static struct prodcons g_uartSendbuffer;
static struct prodcons g_uartRecvbuffer;

/**
*@brief  配置串口
*@param  ttyPS2_fd:串口文件描述符.
         nSpeed:波特率，
         nBits:数据位 7 or 8，
         nEvent:奇偶校验位，
         nStop：停止位
*@return 失败返回-1；成功返回0；
*/

static int set_serial(int ttyPS2_fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newttys1, oldttys1;

    /*保存原有串口配置*/
    if (tcgetattr(ttyPS2_fd, &oldttys1) != 0)
    {
        perror("Setupserial 1");
        return -1;
    }

    memset(&newttys1, 0, sizeof(newttys1));
    /*CREAD 开启串行数据接收,CLOCAL并打开本地连接模式*/
    newttys1.c_cflag |= (CLOCAL | CREAD);

    newttys1.c_cflag &= ~CSIZE; /*设置数据位*/
    switch (nBits)              /*数据位选择*/
    {
    case 7:
        newttys1.c_cflag |= CS7;
        break;
    case 8:
        newttys1.c_cflag |= CS8;
        break;
    default:
        break;
    }

    switch (nEvent) /*奇偶校验位*/
    {
    case '0':
        newttys1.c_cflag |= PARENB;           /*开启奇偶校验*/
        newttys1.c_iflag |= (INPCK | ISTRIP); /*INPCK打开输入奇偶校验，ISTRIP 去除字符的第八个比特*/
        newttys1.c_cflag |= PARODD;           /*启动奇校验（默认为偶校验）*/
        break;
    case 'E':
        newttys1.c_cflag |= PARENB;           /*开启奇偶校验*/
        newttys1.c_iflag |= (INPCK | ISTRIP); /*INPCK打开输入奇偶校验，ISTRIP 去除字符的第八个比特*/
        newttys1.c_cflag &= ~PARODD;          /*启动偶校验*/
        break;
    case 'N':
        newttys1.c_cflag &= ~PARENB; /*无奇偶校验*/
        break;
    default:
        break;
    }

    switch (nSpeed) /*设置波特率*/
    {
    case 2400:
        cfsetispeed(&newttys1, B2400);
        cfsetospeed(&newttys1, B2400);
        break;
    case 4800:
        cfsetispeed(&newttys1, B4800);
        cfsetospeed(&newttys1, B4800);
        break;
    case 9600:
        cfsetispeed(&newttys1, B9600);
        cfsetospeed(&newttys1, B9600);
        break;
    case 19200:
        cfsetispeed(&newttys1, B19200);
        cfsetospeed(&newttys1, B19200);
        break;
    case 115200:
        cfsetispeed(&newttys1, B115200);
        cfsetospeed(&newttys1, B115200);

        break;

    case 1000000:
        cfsetispeed(&newttys1, B1000000);
        cfsetospeed(&newttys1, B1000000);

        break;

    case 2000000:
        cfsetispeed(&newttys1, B2000000);
        cfsetospeed(&newttys1, B2000000);

        break;

    case 3000000:
        cfsetispeed(&newttys1, B3000000);
        cfsetospeed(&newttys1, B3000000);

        break;

    case 4000000:
        cfsetispeed(&newttys1, B4000000);
        cfsetospeed(&newttys1, B4000000);

        break;

    case 3125000:
        // ret = cfsetispeed(&newttys1, 0020001);
        // printf("reti = %d\n",ret);
        // ret = cfsetospeed(&newttys1, 0020001);
        // printf("reto = %d\n",ret);
        newttys1.c_cflag |= 0020001;
        // cfsetispeed(&newttys1, 0020001);
        // cfsetospeed(&newttys1, 0020001);
        break;
    default:
        cfsetispeed(&newttys1, B9600);
        cfsetospeed(&newttys1, B9600);
        break;
    }

    /*设置停止位*/
    /*停止位为1，则清除CSTOPB,如停止位为2，则激活CSTOPB*/
    if (nStop == 1)
    {
        newttys1.c_cflag &= ~CSTOPB; /*默认为停止位1*/
    }
    else if (nStop == 2)
    {
        newttys1.c_cflag |= CSTOPB;
    }

    /*设置最少字符和等待时间，对于接收字符和等待时间没有特别的要求时*/
    newttys1.c_cc[VTIME] = 0; /*非规范模式读取时的超时时间*/
    newttys1.c_cc[VMIN] = 0;  /*非规范模式读取时的最小字符数*/

    /*tcflush 清空终端未完成的输入、输出请求及数据
    TCIFLUSH表示清空正接收到的数据，且不读取出来*/
    tcflush(ttyPS2_fd, TCIFLUSH);

    /*激活配置使其生效*/
    if ((tcsetattr(ttyPS2_fd, TCSANOW, &newttys1)) != 0)
    {
        perror("usart set error");
        return -1;
    }

    return 0;
}
/**
 * @description: ttyPS1初始化
 * @param int argc:
 * @param char *argv:
 * @return NULL
 */
static int uart_ttyPS_init(char *uartx, int baud)
{
    int ret = 0;
    int ttyPS1_fd = open(uartx, O_RDWR | O_NOCTTY); //| O_NDELAY);
    if (ttyPS1_fd < 0)
    {
        printf("Can't open %s port\n", uartx);
        return -1;
    }

    /*可能需要根据情况调整*/
    ret = set_serial(ttyPS1_fd, baud, 8, 'N', 1);

    if (ret < 0)
    {
        printf("set_serial error\n");
        return -1;
    }
    return ttyPS1_fd;
}

// 初始化串口发送和接收Buf
void Init_uart_thread(int nMaxBufSize)
{
    g_uartSendbuffer.nMaxSize = 320 * nMaxBufSize;
    initRingbuf(&g_uartSendbuffer);
    g_uartRecvbuffer.nMaxSize = 320 * nMaxBufSize;
    initRingbuf(&g_uartRecvbuffer);
}

// 把需要发送的音频数据写入串口发送缓冲区
void sendAudioByUart(char *szBuf)
{
    writeRingbuf(&g_uartSendbuffer, szBuf, 320);
}

// 从串口读取320字节语音数据，成功返回1，失败返回0，数据保存在szBuf里
int RecvAudioByUart(char *szBuf)
{

    if (readNoBlockRingbuf(&g_uartRecvbuffer, szBuf, 320))
    {

        return 1;
    }
    return 0;
}

// 从串口发送区读到320字节，写到串口
static int AutoSendWorkStatus()
{
    int i = 0;
    int nLen;
    char src[668];
    if (readNoBlockRingbuf(&g_uartSendbuffer, src, 320))
    {
        write(audioUartFd, src, 320);
    }

    return 0;
}

/**
 * @description:
/**
 * @description:
 * @return NULL
 */
int readSendAudioByUart(char *deviceName)
{

    fd_set fds;
    struct timeval timeout = {0, 10000}; // select等待3秒，3秒轮询，要非阻塞就置0
    int i = 0;
    unsigned char rx_buf[500];
    int maxfdp = 0;
    int nRet = 0;
    int audioLen = 0;
    while (1)
    {

        audioUartFd = uart_ttyPS_init(deviceName, 2000000);
        if (audioUartFd > 0)
            break;
        sleep(3);
    }

    while (1)
    {

        FD_ZERO(&fds);             // 每次循环都要清空集合，否则不能检测描述符变化
        FD_SET(audioUartFd, &fds); // 添加描述符
        maxfdp = audioUartFd + 1;

        switch (select(maxfdp, &fds, NULL, NULL, &timeout)) // select使用
        {
        case -1:
            exit(-1);
            break; // select错误，退出程序
        case 0:
            break; // 再次轮询
        default:
            nRet = read(audioUartFd, rx_buf + audioLen, 320 - audioLen);

            if (nRet)
            {
                audioLen += nRet;
                // 收到320字节写入接收缓冲区
                if (audioLen >= 320)
                {
                    writeRingbuf(&g_uartRecvbuffer, rx_buf, 320);
                    audioLen = 0;
                }
            }

        } // end switch
        AutoSendWorkStatus();

    } // end while
} // end main

// 创建串口发送接收线程
int startAudioThread(char *deviceName)
{

    int ret;
    ret = pthread_create(&tid_rs422, NULL, readSendAudioByUart, deviceName);
    if (ret)
    {
        fprintf(stderr, "startAudioThread Error: %s\n", strerror(ret));
        exit(-1);
    }
    return ret;
}