//
// Created by libowen on 19-3-11.
//

#include "FFmpeg.h"

FFmpeg::FFmpeg(PlayStatus *playStatus, CallJava *callJava, const char *source) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    this->url = source;
    exit = false;
    pthread_mutex_init(&init_mutex,NULL);
    pthread_mutex_init(&seek_mutex,NULL);


}

void *decodecFFmpeg(void *data){

    FFmpeg *fFmpeg = (FFmpeg *)data;
    fFmpeg->decodecFFmpegThread();
    pthread_exit(&fFmpeg->decodecThread);

}

void FFmpeg::prepare() {

    pthread_create(&decodecThread,NULL,decodecFFmpeg,this);

}

int avformat_callback(void * ctx){

    FFmpeg *fFmpeg = (FFmpeg *)(ctx);
    if (fFmpeg->playStatus->exit){
        return AVERROR_EOF;
    }
    return 0;
}

void FFmpeg::decodecFFmpegThread() {
    pthread_mutex_lock(&init_mutex);
    av_register_all();
    avformat_network_init();
    formatContext = avformat_alloc_context();

    formatContext->interrupt_callback.callback = avformat_callback;
    formatContext->interrupt_callback.opaque = this;

    if (avformat_open_input(&formatContext,url,NULL,NULL) != 0){
        LOGE("can not open url");
        callJava->OnError(CHILD_THREAD, 1001, "can not open url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if (avformat_find_stream_info(formatContext,0) < 0){
        LOGE("can not find stream from %s",url);
        callJava->OnError(CHILD_THREAD,1002,"can not find stream from url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            if (audio == NULL){
                audio = new Audio(playStatus, formatContext->streams[i]->codecpar->sample_rate,callJava);
                audio->streamIndex = i;
                audio->codecpar = formatContext->streams[i]->codecpar;
                audio->duration = formatContext->duration / AV_TIME_BASE;
                audio->rational = formatContext->streams[i]->time_base;
                duration = audio->duration;
            }
        }
    }

    AVCodec *codec = avcodec_find_decoder(audio->codecpar->codec_id);
    if (!codec){
        LOGE("can not find codecer");
        callJava->OnError(CHILD_THREAD,1003,"can not find codecer");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    audio->codecContext = avcodec_alloc_context3(codec);

    if (!audio->codecContext){
        LOGE("can not find codecCtx");
        callJava->OnError(CHILD_THREAD,1004,"can not find codecCtx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if (avcodec_parameters_to_context(audio->codecContext,audio->codecpar) < 0){

        LOGE("can not fill decodecctx");
        callJava->OnError(CHILD_THREAD,1005,"can not fill decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if (avcodec_open2(audio->codecContext,codec,0) != 0){
        LOGD("can not open audio streams");
        callJava->OnError(CHILD_THREAD,1006,"can not open audio streams");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if(callJava != NULL)
    {
        if(playStatus != NULL && !playStatus->exit)
        {
            callJava->onPrepare(CHILD_THREAD);
        } else{
            exit = true;
        }
    }
    pthread_mutex_unlock(&init_mutex);


}

void FFmpeg::start() {
    if (audio == NULL){
        LOGE("audio is NULL");
        return;
    }

    audio->play();

    int count = 0;

    while (playStatus != NULL && !playStatus->exit){
        if (playStatus->seek){
            continue;
        }
        if (audio->queue->getQueueSize() > 40){
            continue;
        }
        AVPacket *avPacket = av_packet_alloc();
        pthread_mutex_lock(&seek_mutex);
        int ret = av_read_frame(formatContext, avPacket);
        pthread_mutex_unlock(&seek_mutex);
        if( ret == 0)
        {
            if(avPacket->stream_index == audio->streamIndex)
            {
                //解码操作
                count++;
//                LOGE("解码第 %d 帧", count);
//                av_packet_free(&avPacket);
//                av_free(avPacket);
                audio->queue->putAvPacket(avPacket);

            } else{
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else{
            LOGE("decode finished");
            av_packet_free(&avPacket);
            av_free(avPacket);
            while (playStatus != NULL && !playStatus->exit){
                if (audio->queue->getQueueSize() > 0){
                    continue;
                } else{
                    playStatus->exit = true;
                    break;
                }
            }
        }
    }
    if (callJava != NULL){
        callJava->OnComplete(CHILD_THREAD);
    }
    exit = true;
    //模拟出队
//    while (audio->queue->getQueueSize() > 0)
//    {
//        AVPacket *packet = av_packet_alloc();
//        audio->queue->getAvPacket(packet);
//        av_packet_free(&packet);
//        av_free(packet);
//        packet = NULL;
//    }
//    LOGD("解码完成");
}

FFmpeg::~FFmpeg() {
    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&seek_mutex);
}

void FFmpeg::pause() {
    if (audio != NULL){
        audio->pause();
    }
}

void FFmpeg::resume() {
    if (audio != NULL){
        audio->resume();
    }
}

void FFmpeg::release() {

    if (playStatus->exit){
        return;
    }

    playStatus->exit = true;

    pthread_mutex_lock(&init_mutex);

    int sleepCount = 0;
    while (!exit){
        if (sleepCount > 1000){
            exit = true;
        }
        LOGE("wait ffmpeg  exit %d", sleepCount);
        sleepCount ++;
        av_usleep(1000 * 10);
    }

    if (audio != NULL){
        audio->release();
        delete(audio);
        audio = NULL;
    }
    if (formatContext != NULL){
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        formatContext = NULL;
    }
    if (callJava != NULL){
        callJava = NULL;
    }
    if (playStatus != NULL){
        playStatus = NULL;
    }
    pthread_mutex_unlock(&init_mutex);

}

void FFmpeg::seek(int64_t secds) {

    if (duration <= 0){
        return;
    }
    if (secds > 0 && secds <= duration){
        if (audio != NULL){
            audio->playStatus->seek = true;
            audio->queue->clearAvpacket();
            audio->clock = 0;
            audio->last_time = 0;
            pthread_mutex_lock(&seek_mutex);
            int64_t rel = secds * AV_TIME_BASE;
            avformat_seek_file(formatContext,-1,INT64_MIN,rel,INT64_MAX,0);
            pthread_mutex_unlock(&seek_mutex);
            audio->playStatus->seek = false;
        }
    }
}

void FFmpeg::setVolume(int volume) {
    if (audio != NULL){
        audio->setVolume(volume);
    }
}
