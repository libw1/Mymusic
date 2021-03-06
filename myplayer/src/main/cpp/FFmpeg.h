//
// Created by libowen on 19-3-11.
//

#ifndef MYMUSIC_FFMPEG_H
#define MYMUSIC_FFMPEG_H

#include "CallJava.h"
#include "pthread.h"
#include "Audio.h"
#include "PlayStatus.h"
#include "pthread.h"
#include "Video.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
};

class FFmpeg {

public:
    CallJava *callJava = NULL;
    const char *url = NULL;
    pthread_t decodecThread;
    AVFormatContext *formatContext = NULL;
    Audio *audio = NULL;
    Video *video = NULL;
    PlayStatus *playStatus = NULL;
    pthread_mutex_t init_mutex;
    bool exit = false;
    int duration = 0;
    pthread_mutex_t seek_mutex;

    const AVBitStreamFilter *bsFilter = NULL;
    bool supportMediacodec = false;


public:
    FFmpeg(PlayStatus* playStatus,CallJava *callJava, const char *source);
    ~FFmpeg();

    void prepare();
    void decodecFFmpegThread();
    void start();
    void pause();
    void resume();
    void release();
    void seek(int64_t secds);
    void setVolume(int volume);
    void setMute(int mute);
    void setSpeed(float speed);
    void setPitch(float pitch);
    int getSampleRate();
    void startStopRecord(bool record);
    int getCodecContext(AVCodecParameters *codecpar, AVCodecContext **pCodecContext);
};


#endif //MYMUSIC_FFMPEG_H
