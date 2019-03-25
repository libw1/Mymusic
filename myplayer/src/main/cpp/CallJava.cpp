//
// Created by libowen on 19-3-11.
//

#include "CallJava.h"

CallJava::CallJava(_JavaVM *vm, JNIEnv *jnienv, jobject *job) {
    this->javaVM = vm;
    this->jniEnv = jnienv;
    this->jobj = *job;
    this->jobj = jnienv->NewGlobalRef(jobj);

    jclass jcl = jnienv->GetObjectClass(jobj);
    if (!jcl){
        LOGE("get jclass wrong");
        return;
    }

    this->jmid_prepare = jnienv->GetMethodID(jcl,"onCallPrepare","()V");
    this->jmid_load = jnienv->GetMethodID(jcl, "callOnLoad", "(Z)V");
    this->jmid_onTimeInfo = jnienv->GetMethodID(jcl,"onTimeInfo","(II)V");
}

void CallJava::onPrepare(int type) {

    if (type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj,jmid_prepare);
    } else if (type == CHILD_THREAD){
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env,0) != JNI_OK){
            LOGE("get child thread jnienv wrong");
            return;
        }
        env->CallVoidMethod(jobj,jmid_prepare);
    }

}

void CallJava::onCallLoad(int type, bool load) {

    if (type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj,jmid_load,load);
    } else if (type == CHILD_THREAD){
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env,0) != JNI_OK){
            LOGE("get child thread jnienv wrong");
            return;
        }
        env->CallVoidMethod(jobj,jmid_load,load);
    }

}

void CallJava::onTimeInfo(int type, int current_time, int duration) {

    if (type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj,jmid_onTimeInfo,current_time, duration);
    } else if (type == CHILD_THREAD){
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env,0) != JNI_OK){
            LOGE("get child thread jnienv wrong");
            return;
        }
        env->CallVoidMethod(jobj,jmid_onTimeInfo,current_time, duration);
    }
}
