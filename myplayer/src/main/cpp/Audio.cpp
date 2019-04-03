//
// Created by libowen on 19-3-11.
//

#include "Audio.h"

Audio::Audio(PlayStatus *playStatus, int sampleRate, CallJava *callJava) {

    this->callJava = callJava;
    this->playStatus = playStatus;
    this->sample_rate = sampleRate;
    queue = new Queue(playStatus);
    buffer = (uint8_t *) av_malloc(sampleRate * 2 * 2);

    sampleBuffer = (SAMPLETYPE *)(malloc(sampleRate * 2 * 2));
    soundTouch = new SoundTouch();
    soundTouch->setSampleRate(sampleRate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(pitch);
    soundTouch->setTempo(speed);
}

Audio::~Audio() {

}

void *decodPlay(void *data){
    Audio *audio = (Audio *)(data);
    audio->initOpenSLES();
    pthread_exit(&audio->playThread);
}


void Audio::play() {
    pthread_create(&playThread,NULL,decodPlay,this);

}

int Audio::resampleAudio(void **pcmbuf) {
    data_size = 0;
    while (playStatus != NULL && !playStatus->exit){
        if(queue->getQueueSize() == 0)//加载中
        {
            if(!playStatus->load)
            {
                playStatus->load = true;
                callJava->onCallLoad(CHILD_THREAD, true);
            }
            continue;
        } else{
            if(playStatus->load)
            {
                playStatus->load = false;
                callJava->onCallLoad(CHILD_THREAD, false);
            }
        }
        avPacket = av_packet_alloc();
        if (queue->getAvPacket(avPacket) != 0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        ret = avcodec_send_packet(codecContext,avPacket);
        if (ret != 0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext,avFrame);
        if (ret == 0){
            if (avFrame->channels > 0 && avFrame->channel_layout == 0){
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0){
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            SwrContext *swrContext;
            swrContext = swr_alloc_set_opts(NULL
                    , AV_CH_LAYOUT_STEREO
                    , AV_SAMPLE_FMT_S16
                    , avFrame->sample_rate
                    , avFrame->channel_layout
                    , (AVSampleFormat)(avFrame->format)
                    , avFrame->sample_rate
                    , NULL, NULL);
            if (!swrContext || swr_init(swrContext) < 0){
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                swr_free(&swrContext);
                continue;
            }

            nb = swr_convert(
                    swrContext
                    , &buffer
                    , avFrame->nb_samples
                    , (const uint8_t **)(avFrame->data)
                    , avFrame->nb_samples);

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
//            LOGE("data size is %d", data_size);
            now_time = avFrame->pts * av_q2d(rational);
            if(now_time < clock)
            {
                now_time = clock;
            }
            clock = now_time;

            now_time = avFrame->pts * av_q2d(rational);
            if (now_time < clock){
                now_time = clock;
            }
            clock = now_time;
            *pcmbuf = buffer;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swrContext);
            break;
        } else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
    }
    return data_size;
}

int Audio::getSoundTouchData() {
    while (playStatus != NULL && !playStatus->exit){
        out_buffer = NULL;
        if (finished){
            finished = false;
            data_size = resampleAudio(reinterpret_cast<void **>(&out_buffer));
            if (data_size > 0){
                for (int i = 0; i < data_size / 2 + 1; i++) {
                    sampleBuffer[i] = (out_buffer[i * 2] | ((out_buffer[i * 2 + 1]) << 8));
                }
                soundTouch->putSamples(sampleBuffer,nb);
                num = soundTouch->receiveSamples(sampleBuffer,data_size / 4);
            } else{
                soundTouch->flush();
            }
        }
        if (num == 0){
            finished = true;
            continue;
        } else{
            if (out_buffer == NULL){
                num = soundTouch->receiveSamples(sampleBuffer,data_size / 4);
            }
            if (num == 0){
                finished = true;
                continue;
            }
            return num;
        }
    }
    return 0;
}

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void * context)
{
    Audio *audio = (Audio *) context;
    if(audio != NULL)
    {
        int buffersize = audio->getSoundTouchData();
        if(buffersize > 0)
        {
//            LOGD("pcmBufferCallBack %d",buffersize);
            audio->clock += buffersize / (double)(audio->sample_rate * 2 * 2);
            if (audio->clock - audio->last_time > 0.1){

                audio->last_time = audio->clock;

                audio->callJava->onTimeInfo(CHILD_THREAD,audio->clock,audio->duration);
            }
            audio->callJava->onPcmToAac(CHILD_THREAD,buffersize * 4,audio->sampleBuffer);
            audio->callJava->OnPCMDB(CHILD_THREAD,
            audio->getPCMDB(reinterpret_cast<char *>(audio->sampleBuffer), buffersize * 2 * 2));
            (* audio-> pcmBufferQueue)->Enqueue( audio->pcmBufferQueue, (char *) audio-> sampleBuffer, buffersize * 2 *2);
        }
    }
}

void Audio::initOpenSLES() {

    SLresult result;
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void)result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void)result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, 0};


    // 第三步，配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};

    SLDataFormat_PCM pcm={
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            static_cast<SLuint32>(getCurrentSampleRateForOpensles(sample_rate)),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};


    const SLInterfaceID ids[4] = {SL_IID_BUFFERQUEUE,SL_IID_VOLUME,SL_IID_PLAYBACKRATE,SL_IID_MUTESOLO};
    const SLboolean req[4] = {SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 4, ids, req);
    //初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

//    得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmVolumePlay);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmPlayerMuteSolo);

//    注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);

    setVolume(volumePercent);
    setMute(mute);

    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
//    获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    pcmBufferCallBack(pcmBufferQueue, this);
}

int Audio::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate)
    {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate =  SL_SAMPLINGRATE_44_1;
            break;
    }
    return rate;
}

void Audio::pause() {
    if (pcmPlayerPlay != NULL){
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,SL_PLAYSTATE_PAUSED);
    }
}

void Audio::resume() {
    if (pcmPlayerPlay != NULL){
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,SL_PLAYSTATE_PLAYING);
    }
}

void Audio::stop() {
    if (pcmPlayerPlay != NULL){
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,SL_PLAYSTATE_STOPPED);
    }

}

void Audio::release() {
    stop();
    if (queue != NULL){
        delete(queue);
        queue = NULL;
    }
    if (pcmPlayerObject != NULL){
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
//        pcmVolumePlay = NULL;
    }
    if (outputMixObject != NULL){
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }
    if (engineObject != NULL){
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    if (buffer != NULL){
        free(buffer);
        buffer = NULL;
    }
    if (codecContext != NULL){
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        codecContext = NULL;
    }
    if (callJava != NULL){
        callJava = NULL;
    }
    if (playStatus != NULL){
        playStatus = NULL;
    }
}

void Audio::setVolume(int percent) {

    volumePercent = percent;
    if(pcmVolumePlay != NULL)
    {
        if(percent > 30)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -20);
        }
        else if(percent > 25)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -22);
        }
        else if(percent > 20)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -25);
        }
        else if(percent > 15)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -28);
        }
        else if(percent > 10)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -30);
        }
        else if(percent > 5)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -34);
        }
        else if(percent > 3)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -37);
        }
        else if(percent > 0)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -40);
        }
        else{
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -100);
        }
    }
}

void Audio::setMute(int mu) {
    mute = mu;
    if (pcmPlayerMuteSolo != NULL) {
        if (mute == 0) {
            (*pcmPlayerMuteSolo)->SetChannelMute(pcmPlayerMuteSolo,0, true);
            (*pcmPlayerMuteSolo)->SetChannelMute(pcmPlayerMuteSolo,1, false);
        } else if(mute == 1){
            (*pcmPlayerMuteSolo)->SetChannelMute(pcmPlayerMuteSolo,0, false);
            (*pcmPlayerMuteSolo)->SetChannelMute(pcmPlayerMuteSolo,1, true);
        } else if (mute == 2){
            (*pcmPlayerMuteSolo)->SetChannelMute(pcmPlayerMuteSolo,0, false);
            (*pcmPlayerMuteSolo)->SetChannelMute(pcmPlayerMuteSolo,1, false);
        }
    }
}

void Audio::setSpeed(float speed) {
    if (soundTouch != NULL){
        soundTouch->setTempo(speed);
    }
}

void Audio::setPitch(float pitch) {

    if (soundTouch != NULL){
        soundTouch->setPitch(pitch);
    }
}

int Audio::getPCMDB(char *pcmchar, size_t pcmsize) {
    int db = 0;
    short int pcmvalue = 0;
    double sum = 0;
    for (int i = 0; i < pcmsize; i += 2) {
        memcpy(&pcmvalue,pcmchar + i,2);
        sum += abs(pcmvalue);
    }
    sum = sum / pcmsize / 2;
    if (sum > 0){
        db = (int)20.0 * log10(sum);
    }
    return db;
}

