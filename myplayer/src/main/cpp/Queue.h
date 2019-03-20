//
// Created by libowen on 19-3-20.
//

#ifndef MYMUSIC_QUEUE_H
#define MYMUSIC_QUEUE_H

#include "queue"
#include "PlayStatus.h"
#include "pthread.h"
#include "AndroidLog.h"

extern "C"{
#include "libavcodec/avcodec.h"
};

class Queue {

public:
    std::queue<AVPacket *> queuePacket;
    PlayStatus *playStatus = NULL;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;

public:
    Queue(PlayStatus *playStatus);
    ~Queue();

    int putAvPacket(AVPacket * avPacket);
    int getAvPacket(AVPacket * avPacket);
    int getQueueSize();
};


#endif //MYMUSIC_QUEUE_H
