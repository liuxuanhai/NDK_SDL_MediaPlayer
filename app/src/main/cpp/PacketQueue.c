//
// Created by xiang on 2017-4-8.
//

#include "PacketQueue.h"

void packet_queue_init(PacketQueue *q) {
    memset(q, 0, sizeof(PacketQueue));
    q->mutex = SDL_CreateMutex();
    q->cond = SDL_CreateCond();
    q->enable = 1;
}

int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
    AVPacketList *pkt1;
    int ret;
    SDL_LockMutex(q->mutex);
    for (; ;) {
        if (q->enable == 0) {
            ret = -1;
            break;
        }
        pkt1 = q->first_pkt;
        if (pkt1) {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt) {
                q->last_pkt = NULL;
            }
            q->nb_packets--;
            q->size -= pkt1->pkt.size;
            *pkt = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {   //指定在无数据的时候是否阻塞线程等待
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);//在队列无数据的时候，设定为阻塞等待则
        }
    }

    SDL_UnlockMutex(q->mutex);

    return ret;
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
    AVPacketList *pkt1;
    pkt1 = (AVPacketList *) av_malloc(sizeof(AVPacketList));
    if (!pkt1) {
        return -1;
    }
    pkt1->pkt = *pkt;
    pkt1->next = NULL;
    SDL_LockMutex(q->mutex);//进行lock操作
    if (!q->last_pkt) {
        q->first_pkt = pkt1;
    } else {
        q->last_pkt->next = pkt1;
    }
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size;
    SDL_CondSignal(q->cond);//通知取数据的线程，已有新插入的数据
    SDL_UnlockMutex(q->mutex);
    return 0;
}