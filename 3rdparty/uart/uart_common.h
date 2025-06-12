#ifndef __UART_COMMMON__H
#define __UART_COMMMON__H

extern "C"
{
    /*开辟串口缓冲区
     *nMaxBufSize 接收发送缓冲区大小
     */
    void Init_uart_thread(int nMaxBufSize);

    /*起接收发送语音线程
     *deviceName 串口号
     */
    int startAudioThread(char *deviceName);

    /*发送一段语音数据长度320字节
     */
    void sendAudioByUart(char *szBuf);

    /*接收一段语音数据长度320字节
    返回值0：未收到语音  1收到320字节语音
    */
    int RecvAudioByUart(char *szBuf);
}
#endif