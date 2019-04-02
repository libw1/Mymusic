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
    this->jmid_onError = jnienv->GetMethodID(jcl,"onError","(ILjava/lang/String;)V");
    this->jmid_onComplete = jnienv->GetMethodID(jcl,"onComplete","()V");
    this->jmid_onPCMDB = jnienv->GetMethodID(jcl,"onPCMDB","(I)V");
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
        javaVM->DetachCurrentThread();
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
        javaVM->DetachCurrentThread();
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
        javaVM->DetachCurrentThread();
    }
}

CallJava::~CallJava() {

}

void CallJava::OnError(int type, int code, char *msg) {
    if (type == MAIN_THREAD)
    {
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj,jmid_onError,code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
    } else if (type == CHILD_THREAD){
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env,0) != JNI_OK){
            LOGE("get child thread jnienv wrong");
            return;
        }
        jstring jmsg = env->NewStringUTF(msg);
        env->CallVoidMethod(jobj,jmid_onError,code, jmsg);
        env->DeleteLocalRef(jmsg);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::OnComplete(int type) {
    if (type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj,jmid_onComplete);
    } else if (type == CHILD_THREAD){
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env,0) != JNI_OK){
            LOGE("get child thread jnienv wrong");
            return;
        }
        env->CallVoidMethod(jobj,jmid_onComplete);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::OnPCMDB(int type, int db) {

    if (type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj,jmid_onPCMDB,db);
    } else if (type == CHILD_THREAD){
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env,0) != JNI_OK){
            LOGE("get child thread jnienv wrong");
            return;
        }
        env->CallVoidMethod(jobj,jmid_onPCMDB,db);
        javaVM->DetachCurrentThread();
    }
}
