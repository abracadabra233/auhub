

#include "ringbuf.h"
/* 初始化缓冲区结构 */
void initRingbuf(struct prodcons *b)
{
    pthread_mutex_init(&b->lock, NULL);
    pthread_cond_init(&b->notempty, NULL);
    pthread_cond_init(&b->notfull, NULL);
    b->readpos = 0;
    b->nDatalen = 0;
    b->writepos = 0;
    b->buffer = malloc(b->nMaxSize);
}
/* 将产品放入缓冲区,这里是存入一个整数*/
void writeRingbuf(struct prodcons *b, char *data, int len)
{
    pthread_mutex_lock(&b->lock);
    int i = 0;
    /* 等待缓冲区未满*/

    if ((b->nDatalen + len) > b->nMaxSize)
    {
        // printf("wait write %d %d %d\n", b->nDatalen , len, b->nMaxSize);
        pthread_cond_wait(&b->notfull, &b->lock);
    }
    // printf("write %02x :%02x\n",data[18], data[19]);
    /* 写数据,并移动指针 */
    for (i = 0; i < len; i++)
    {
        b->buffer[b->writepos] = data[i];
        b->writepos++;
        if (b->writepos >= b->nMaxSize)
            b->writepos = 0;
    }
    b->nDatalen += len;
    /* 设置缓冲区非空的条件变量*/
    pthread_cond_signal(&b->notempty);
    pthread_mutex_unlock(&b->lock);
}

/* 从缓冲区中取出整数*/
void readRingbuf(struct prodcons *b, char *dst, int len)
{

    pthread_mutex_lock(&b->lock);
    int i = 0;

    /* 等待缓冲区非空*/
    while (b->nDatalen < len)
    {

        pthread_cond_wait(&b->notempty, &b->lock);
    }
    /* 读数据,移动读指针*/
    for (i = 0; i < len; i++)
    {
        dst[i] = b->buffer[b->readpos];
        b->readpos++;
        if (b->readpos >= b->nMaxSize)
            b->readpos = 0;
    }
    b->nDatalen -= len;

    // printf("read %02x :%02x\n",dst[18], dst[19]);
    /* 设置缓冲区未满的条件变量*/
    pthread_cond_signal(&b->notfull);
    pthread_mutex_unlock(&b->lock);
    return;
}
int readNoBlockRingbuf(struct prodcons *b, char *dst, int len)
{

    pthread_mutex_lock(&b->lock);
    int i = 0;

    /* 等待缓冲区非空*/
    if (b->nDatalen < len)
    {
        pthread_mutex_unlock(&b->lock);
        return 0;
    }
    /* 读数据,移动读指针*/
    for (i = 0; i < len; i++)
    {
        dst[i] = b->buffer[b->readpos];
        b->readpos++;
        if (b->readpos >= b->nMaxSize)
            b->readpos = 0;
    }
    b->nDatalen -= len;

    // printf("read %02x :%02x\n",dst[18], dst[19]);
    /* 设置缓冲区未满的条件变量*/
    pthread_cond_signal(&b->notfull);
    pthread_mutex_unlock(&b->lock);
    return len;
}
