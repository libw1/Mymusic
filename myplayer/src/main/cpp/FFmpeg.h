//
// Created by libowen on 19-3-11.
//

#ifndef MYMUSIC_FFMPEG_H
#define MYMUSIC_FFMPEG_H

#include "CallJava.h"
#include "pthread.h"
#include "Audio.h"
#include "PlayStatus.h"

extern "C"{
#include <libavformat/avformat.h>
};

class FFmpeg {

public:
    CallJava *callJava = NULL;
    const char *url = NULL;
    pthread_t decodecThread;
    AVFormatContext *formatContext = NULL;
    Audio *audio = NULL;
    PlayStatus *playStatus = NULL;



public:
    FFmpeg(PlayStatus* playStatus,CallJava *callJava, const char *source);
    ~FFmpeg();

    void prepare();
    void decodecFFmpegThread();
    void start();
    void pause();
    void resume();
};


#endif //MYMUSIC_FFMPEG_H
