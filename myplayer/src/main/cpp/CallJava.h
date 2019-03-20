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

public:
    CallJava(_JavaVM *vm, JNIEnv *jnienv, jobject *jobj);
    ~CallJava();

    void onPrepare(int type);
};


#endif //MYMUSIC_CALLJAVA_H
