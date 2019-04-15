//
// Created by chen-gui on 19-4-10.
//

#ifndef MYMUSIC_VIDEO_H
#define MYMUSIC_VIDEO_H


#include "Queue.h"
#include "CallJava.h"
#include "pthread.h"
#include "Audio.h"

#define CODEC_YUV 0
#define CODEC_MEDIA_CODEC 1

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
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

    Audio *audio = NULL;
    double clock = 0;
    double delayTime = 0;
    double defaultDelayTime = 0.04;
    pthread_mutex_t codecMutex;

    int codecType = CODEC_YUV;
    AVBSFContext *abs_ctx = NULL;

public:
    Video(PlayStatus *playStatus,CallJava *callJava);
    ~Video();

    void play();
    void release();

    double getFrameDiffTime(AVFrame *avFrame);

    double getDelayTime(double diff);
};


#endif //MYMUSIC_VIDEO_H
