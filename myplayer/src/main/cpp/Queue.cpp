//
// Created by libowen on 19-3-20.
//

#include "Queue.h"

Queue::Queue(PlayStatus *playStatus) {
    this->playStatus = playStatus;
    pthread_mutex_init(&mutexPacket,NULL);
    pthread_cond_init(&condPacket,NULL);
}

Queue::~Queue() {
    clearAvpacket();
}

int Queue::putAvPacket(AVPacket *avPacket) {
    pthread_mutex_lock(&mutexPacket);
    queuePacket.push(avPacket);
//    LOGD("放入一个AVpacket到队里里面， 个数为：%d", queuePacket.size());
    pthread_cond_signal(&condPacket);
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int Queue::getAvPacket(AVPacket *avPacket) {
    pthread_mutex_lock(&mutexPacket);
    while (playStatus != NULL && !playStatus->exit){
        if (queuePacket.size() > 0){
            AVPacket *packet = queuePacket.front();
            if (av_packet_ref(avPacket,packet) == 0){
                queuePacket.pop();
            }
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
//            LOGD("从队列里面取出一个AVpacket，还剩下 %d 个", queuePacket.size());
            break;
        } else{
            pthread_cond_wait(&condPacket,&mutexPacket);
        }
    }

    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int Queue::getQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutexPacket);
    size = queuePacket.size();
    pthread_mutex_unlock(&mutexPacket);
    return size;
}

void Queue::clearAvpacket() {
    pthread_cond_signal(&condPacket);
    pthread_mutex_lock(&mutexPacket);

    while (!queuePacket.empty()){
        AVPacket *avPacket = queuePacket.front();
        queuePacket.pop();
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }
    pthread_mutex_unlock(&mutexPacket);

}
