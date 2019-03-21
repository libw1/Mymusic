//
// Created by libowen on 19-3-11.
//

#include "FFmpeg.h"

FFmpeg::FFmpeg(PlayStatus *playStatus, CallJava *callJava, const char *source) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    this->url = source;


}

void *decodecFFmpeg(void *data){

    FFmpeg *fFmpeg = (FFmpeg *)data;
    fFmpeg->decodecFFmpegThread();
    pthread_exit(&fFmpeg->decodecThread);

}

void FFmpeg::prepare() {

    pthread_create(&decodecThread,NULL,decodecFFmpeg,this);

}

void FFmpeg::decodecFFmpegThread() {
    av_register_all();
    avformat_network_init();
    formatContext = avformat_alloc_context();

    if (avformat_open_input(&formatContext,url,NULL,NULL) != 0){
        LOGE("can not open url")
        return;
    }

    if (avformat_find_stream_info(formatContext,0) < 0){
        LOGE("can not find stream from %s",url);
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            if (audio == NULL){
                audio = new Audio(playStatus, formatContext->streams[i]->codecpar->sample_rate);
                audio->streamIndex = i;
                audio->codecpar = formatContext->streams[i]->codecpar;
            }
        }
    }

    AVCodec *codec = avcodec_find_decoder(audio->codecpar->codec_id);
    if (!codec){
        LOGE("can not find codecer");
        return;
    }

    audio->codecContext = avcodec_alloc_context3(codec);

    if (!audio->codecContext){
        LOGE("can not find codecCtx");
        return;
    }

    if (avcodec_parameters_to_context(audio->codecContext,audio->codecpar) < 0){

        LOGE("can not fill decodecctx");
        return;
    }

    if (avcodec_open2(audio->codecContext,codec,0) != 0){
        LOGD("can not open audio streams");
        return;
    }

    callJava->onPrepare(CHILD_THREAD);


}

void FFmpeg::start() {
    if (audio == NULL){
        LOGE("audio is NULL");
        return;
    }

    audio->play();

    int count = 0;

    while (playStatus != NULL && !playStatus->exit){
        AVPacket *avPacket = av_packet_alloc();
        if(av_read_frame(formatContext, avPacket) == 0)
        {
            if(avPacket->stream_index == audio->streamIndex)
            {
                //解码操作
                count++;
                LOGE("解码第 %d 帧", count);
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
    //模拟出队
//    while (audio->queue->getQueueSize() > 0)
//    {
//        AVPacket *packet = av_packet_alloc();
//        audio->queue->getAvPacket(packet);
//        av_packet_free(&packet);
//        av_free(packet);
//        packet = NULL;
//    }
    LOGD("解码完成");
}

FFmpeg::~FFmpeg() {

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

