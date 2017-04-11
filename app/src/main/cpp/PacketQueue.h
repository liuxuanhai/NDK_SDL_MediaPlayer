//
// Created by xiang on 2017-4-8.
//

#ifndef PLAYER_PACKETQUEUE_H
#define PLAYER_PACKETQUEUE_H

#include <libavformat/avformat.h>
#include <SDL_mutex.h>

/*
 * AVPacket队列
 */
typedef struct PacketQueue {
    AVPacketList *first_pkt, *last_pkt;   // 队头,队尾
    int nb_packets;   //包的个数
    int size;     // 占用空间的字节数
    SDL_mutex *mutex;   // 互斥信号量
    SDL_cond *cond;    // 条件变量
    int enable;     //标识该列队是否在用
} PacketQueue;

//队列初始化
void packet_queue_init(PacketQueue *q);

//从列队中获取一个包
int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);

//向列队中放入一个包
int packet_queue_put(PacketQueue *q, AVPacket *pkt);

#endif //PLAYER_PACKETQUEUE_H
