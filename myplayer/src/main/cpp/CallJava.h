//
// Created by libowen on 19-3-11.
//

#ifndef MYMUSIC_CALLJAVA_H
#define MYMUSIC_CALLJAVA_H

#define MAIN_THREAD 0
#define CHILD_THREAD 1

#include <jni.h>
#include "AndroidLog.h"

class CallJava {
public:
    _JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;

    jmethodID jmid_prepare;
    jmethodID jmid_load;
    jmethodID jmid_onTimeInfo;
    jmethodID jmid_onError;
    jmethodID jmid_onComplete;
    jmethodID jmid_onPCMDB;
    jmethodID jmid_onPcmToAac;
    jmethodID jmid_onRenderYUV;
    jmethodID jmid_onSupportCodec;

public:
    CallJava(_JavaVM *vm, JNIEnv *jnienv, jobject *jobj);
    ~CallJava();

    void onPrepare(int type);

    void onCallLoad(int type, bool load);

    void onTimeInfo(int type, int current_time, int duration);

    void OnError(int type, int code, char *msg);

    void OnComplete(int type);

    void OnPCMDB(int type, int db);

    void onPcmToAac(int type, int size, void *buf);

    void onCallRenderYUV(int width,int height,uint8_t *fy,uint8_t *fu,uint8_t *fv);

    bool isSupportCodec(const char *codec);
};


#endif //MYMUSIC_CALLJAVA_H
