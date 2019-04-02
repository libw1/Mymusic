//
// Created by libowen on 19-3-11.
//

#ifndef MYMUSIC_AUDIO_H
#define MYMUSIC_AUDIO_H

#include "PlayStatus.h"
#include "Queue.h"
#include "pthread.h"
#include "CallJava.h"
#include "SoundTouch.h"

using namespace soundtouch;

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
    CallJava *callJava = NULL;

    pthread_t playThread;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = 0;
    uint8_t *buffer = NULL;
    int data_size = 0;
    int sample_rate = 0;

    int duration = 0;
    AVRational rational;
    double clock;
    double now_time;
    double last_time;
    int volumePercent = 100;
    int mute = 2;
    float speed = 1.0f;
    float pitch = 1.0f;

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
    SLVolumeItf pcmVolumePlay = NULL;
    SLMuteSoloItf pcmPlayerMuteSolo = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

    //Sound Touch
    SoundTouch *soundTouch = NULL;
    SAMPLETYPE *sampleBuffer = NULL;
    bool finished = true;
    uint8_t *out_buffer = NULL;
    int nb = 0;
    int num = 0;

public:
    Audio(PlayStatus *playStatus, int sampleRate, CallJava *callJava);
    ~Audio();

    void play();
    int resampleAudio(void **pcmbuf);

    void initOpenSLES();
    int getCurrentSampleRateForOpensles(int sample_rate);

    void pause();

    void resume();

    void stop();

    void release();

    void setVolume(int volume);

    void setMute(int mute);

    int getSoundTouchData();

    void setSpeed(float speed);

    void setPitch(float pitch);

    int getPCMDB(char *pcmchar, size_t pcmsize);
};


#endif //MYMUSIC_AUDIO_H
