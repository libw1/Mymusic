//
// Created by chen-gui on 19-4-10.
//

#ifndef MYMUSIC_VIDEO_H
#define MYMUSIC_VIDEO_H


#include "Queue.h"
#include "CallJava.h"
#include "pthread.h"

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
};
class Video {

public:

    int streamIndex = -1;
    AVCodecContext *codecContext = NULL;
    AVCodecParameters *codecpar = NULL;
    Queue *queue = NULL;
    PlayStatus *playStatus = NULL;
    CallJava *callJava = NULL;

    pthread_t playThread;
    AVRational rational;

public:
    Video(PlayStatus *playStatus,CallJava *callJava);
    ~Video();

    void play();
    void release();
};


#endif //MYMUSIC_VIDEO_H
