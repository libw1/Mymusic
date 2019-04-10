//
// Created by chen-gui on 19-4-10.
//

#include "Video.h"

Video::Video(PlayStatus *playStatus, CallJava *callJava) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    this->queue = new Queue(playStatus);
}

Video::~Video() {

}

void *playVideo(void *data){
    Video *video = static_cast<Video *>(data);
    while (video->playStatus != NULL && !video->playStatus->exit){
        if (video->playStatus->seek){
            av_usleep(1000 * 100);
            continue;
        }
        if (video->queue->getQueueSize() == 0){
            if (!video->playStatus->load){
                video->playStatus->load = true;
                video->callJava->onCallLoad(CHILD_THREAD,true);
            }
            av_usleep(1000 * 100);
            continue;
        } else {
            if (video->playStatus->load){
                video->playStatus->load = true;
                video->callJava->onCallLoad(CHILD_THREAD,false);
            }
        }

        AVPacket *avPacket = av_packet_alloc();
        if (video->queue->getAvPacket(avPacket) != 0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        if (avcodec_send_packet(video->codecContext,avPacket) != 0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        AVFrame *avFrame = av_frame_alloc();
        if (avcodec_receive_frame(video->codecContext,avFrame) != 0){
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        LOGE("解码一个avpacket");
        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;

    }
    pthread_exit(&video->playThread);
}

void Video::play() {

    pthread_create(&playThread,NULL,playVideo,this);

}

void Video::release() {
    if (queue != NULL){
        delete queue;
        queue = NULL;
    }

    if (codecContext != NULL){
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        codecContext = NULL;
    }
    if (playStatus != NULL){
        playStatus = NULL;
    }
    if (callJava != NULL){
        callJava = NULL;
    }

}
