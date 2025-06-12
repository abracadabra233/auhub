
#ifndef RING_BUF__H
#define RING_BUF__H
#include <stdio.h>
#include <pthread.h>
#define BUFFER_SIZE 4096 // 缓冲区数量
struct prodcons
{
    // 缓冲区相关数据结构
    char *buffer;          /* 实际数据存放的数组*/
    pthread_mutex_t lock;  /* 互斥体lock 用于对缓冲区的互斥操作 */
    int readpos, writepos; /* 读写指针*/
    int nDatalen;
    int nMaxSize;
    pthread_cond_t notempty; /* 缓冲区非空的条件变量 */
    pthread_cond_t notfull;  /* 缓冲区未满的条件变量 */
};

void initRingbuf(struct prodcons *b);
void writeRingbuf(struct prodcons *b, char *data, int len);
void readRingbuf(struct prodcons *b, char *data, int len);
#endif