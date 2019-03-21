//
// Created by libowen on 19-3-11.
//

#ifndef MYMUSIC_AUDIO_H
#define MYMUSIC_AUDIO_H

#include "PlayStatus.h"
#include "Queue.h"
#include "pthread.h"

extern "C"{
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};

class Audio {

public:
    int streamIndex = -1;
    AVCodecContext *codecContext = NULL;
    AVCodecParameters *codecpar = NULL;
    Queue *queue = NULL;
    PlayStatus *playStatus = NULL;

    pthread_t playThread;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = 0;
    uint8_t *buffer = NULL;
    int data_size = 0;
    int sample_rate = 0;

    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;


public:
    Audio(PlayStatus *playStatus, int sampleRate);
    ~Audio();

    void play();
    int resampleAudio();

    void initOpenSLES();
    int getCurrentSampleRateForOpensles(int sample_rate);

    void pause();

    void resume();
};


#endif //MYMUSIC_AUDIO_H
