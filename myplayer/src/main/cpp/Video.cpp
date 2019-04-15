//
// Created by chen-gui on 19-4-10.
//

#include "Video.h"

Video::Video(PlayStatus *playStatus, CallJava *callJava) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    this->queue = new Queue(playStatus);

    pthread_mutex_init(&codecMutex, NULL);
}

Video::~Video() {
    pthread_mutex_destroy(&codecMutex);
}

void *playVideo(void *data){
    Video *video = static_cast<Video *>(data);
    while (video->playStatus != NULL && !video->playStatus->exit){
        if (video->playStatus->seek){
            av_usleep(1000 * 100);
            continue;
        }
        if (video->playStatus->pause){
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
        if (video->codecType == CODEC_MEDIA_CODEC){
            if(av_bsf_send_packet(video->abs_ctx, avPacket) != 0)
            {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }
            while(av_bsf_receive_packet(video->abs_ctx, avPacket) == 0)
            {
                LOGE("开始解码");

                double diff = video->getFrameDiffTime(NULL, avPacket);
                LOGE("diff is %f", diff);

                av_usleep(video->getDelayTime(diff) * 1000000);
                video->callJava->onCallDecodeAVPacket(avPacket->size, avPacket->data);

                av_packet_free(&avPacket);
                av_free(avPacket);
                continue;
            }
            avPacket = NULL;

        } else if(video->codecType == CODEC_YUV){
            pthread_mutex_lock(&video->codecMutex);
            if (avcodec_send_packet(video->codecContext,avPacket) != 0){
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&video->codecMutex);
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
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }
            if (avFrame->format == AV_PIX_FMT_YUV420P){
                LOGD("当前是YUV420P");
                double diff = video->getFrameDiffTime(avFrame,NULL);
                LOGD("diff is %f",diff);

                av_usleep(video->getDelayTime(diff) * 1000000);


                video->callJava->onCallRenderYUV(video->codecContext->width,
                                                 video->codecContext->height,
                                                 avFrame->data[0],
                                                 avFrame->data[1],
                                                 avFrame->data[2]);
            } else{
                LOGD("当前不是YUV420P");

                AVFrame *frameYUV420P = av_frame_alloc();
                int num = av_image_get_buffer_size(
                        AV_PIX_FMT_YUV420P,
                        video->codecContext->width,
                        video->codecContext->height,
                        1);

                uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));

                av_image_fill_arrays(frameYUV420P->data,
                                     frameYUV420P->linesize,
                                     buffer,
                                     AV_PIX_FMT_YUV420P,
                                     video->codecContext->width,
                                     video->codecContext->height,
                                     1);

                SwsContext *swsContext = sws_getContext(video->codecContext->width,
                                                        video->codecContext->height,
                                                        video->codecContext->pix_fmt,
                                                        video->codecContext->width,
                                                        video->codecContext->height,
                                                        AV_PIX_FMT_YUV420P,
                                                        SWS_BICUBIC,NULL,NULL,NULL);

                if (!swsContext){
                    av_frame_free(&frameYUV420P);
                    av_free(frameYUV420P);
                    av_free(buffer);
                    pthread_mutex_unlock(&video->codecMutex);
                    continue;
                }

                sws_scale(swsContext,
                          reinterpret_cast<const uint8_t *const *>(avFrame->data),
                          avFrame->linesize,
                          0,
                          avFrame->height,
                          frameYUV420P->data,
                          frameYUV420P->linesize);

                double diff = video->getFrameDiffTime(avFrame,NULL);
                LOGD("diff is %f",diff);

                av_usleep(video->getDelayTime(diff) * 1000000);
                video->callJava->onCallRenderYUV(video->codecContext->width,
                                                 video->codecContext->height,
                                                 frameYUV420P->data[0],
                                                 frameYUV420P->data[1],
                                                 frameYUV420P->data[2]);

                av_frame_free(&frameYUV420P);
                av_free(frameYUV420P);
                av_free(buffer);
                sws_freeContext(swsContext);
            }

//        LOGE("解码一个avpacket");
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            pthread_mutex_unlock(&video->codecMutex);
        }

    }
    return 0;
}

void Video::play() {

    if (playStatus != NULL && !playStatus->exit){
        pthread_create(&playThread,NULL,playVideo,this);
    }

}

void Video::release() {

    if(queue != NULL)
    {
        queue->noticeQueue();
    }
    pthread_join(playThread, NULL);

    if (queue != NULL){
        delete queue;
        queue = NULL;
    }

    if(abs_ctx != NULL)
    {
        av_bsf_free(&abs_ctx);
        abs_ctx = NULL;
    }
    if (codecContext != NULL){
        pthread_mutex_lock(&codecMutex);
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        codecContext = NULL;
        pthread_mutex_unlock(&codecMutex);
    }
    if (playStatus != NULL){
        playStatus = NULL;
    }
    if (callJava != NULL){
        callJava = NULL;
    }

}

double Video::getFrameDiffTime(AVFrame *avFrame, AVPacket *avPacket) {
    double pts = 0;
    if(avFrame != NULL)
    {
        pts = av_frame_get_best_effort_timestamp(avFrame);
    }
    if(avPacket != NULL)
    {
        pts = avPacket->pts;
    }
    if(pts == AV_NOPTS_VALUE)
    {
        pts = 0;
    }
    pts *= av_q2d(rational);

    if(pts > 0)
    {
        clock = pts;
    }

    double diff = audio->clock - clock;
    return diff;
}

double Video::getDelayTime(double diff) {
    if(diff > 0.003)
    {
        delayTime = delayTime * 2 / 3;
        if(delayTime < defaultDelayTime / 2)
        {
            delayTime = defaultDelayTime * 2 / 3;
        }
        else if(delayTime > defaultDelayTime * 2)
        {
            delayTime = defaultDelayTime * 2;
        }
    }
    else if(diff < - 0.003)
    {
        delayTime = delayTime * 3 / 2;
        if(delayTime < defaultDelayTime / 2)
        {
            delayTime = defaultDelayTime * 2 / 3;
        }
        else if(delayTime > defaultDelayTime * 2)
        {
            delayTime = defaultDelayTime * 2;
        }
    }
    else if(diff == 0.003)
    {

    }
    if(diff >= 0.5)
    {
        delayTime = 0;
    }
    else if(diff <= -0.5)
    {
        delayTime = defaultDelayTime * 2;
    }

    if(fabs(diff) >= 10)
    {
        delayTime = defaultDelayTime;
    }
    return delayTime;
}
