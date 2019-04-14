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
    this->jmid_onPcmToAac = jnienv->GetMethodID(jcl,"encodecPcmToAcc","(I[B)V");
    this->jmid_onRenderYUV = jnienv->GetMethodID(jcl,"onCallRenderYUV","(II[B[B[B)V");
    this->jmid_onSupportCodec = jnienv->GetMethodID(jcl, "onCallSupportMediaCodec", "(Ljava/lang/String;)Z");
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

void CallJava::onPcmToAac(int type, int size, void *buf) {

    if (type == MAIN_THREAD)
    {
        jbyteArray jbyteArray = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jbyteArray, 0, size, static_cast<const jbyte *>(buf));
        jniEnv->CallVoidMethod(jobj,jmid_onPcmToAac,size,jbyteArray);
        jniEnv->DeleteLocalRef(jbyteArray);
    } else if (type == CHILD_THREAD){
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env,0) != JNI_OK){
            LOGE("get child thread jnienv wrong");
            return;
        }
        jbyteArray jbyteArray = env->NewByteArray(size);
        env->SetByteArrayRegion(jbyteArray, 0, size, static_cast<const jbyte *>(buf));
        env->CallVoidMethod(jobj,jmid_onPcmToAac,size,jbyteArray);
        env->DeleteLocalRef(jbyteArray);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv) {
    JNIEnv *env;
    if (javaVM->AttachCurrentThread(&env,0) != JNI_OK){
        LOGE("get child thread jnienv wrong");
        return;
    }
    jbyteArray y = env->NewByteArray(width * height);
    env->SetByteArrayRegion(y, 0, width * height, reinterpret_cast<const jbyte *>(fy));
    jbyteArray u = env->NewByteArray(width * height / 4);
    env->SetByteArrayRegion(u, 0, width * height / 4, reinterpret_cast<const jbyte *>(fu));
    jbyteArray v = env->NewByteArray(width * height / 4);
    env->SetByteArrayRegion(v, 0, width * height / 4, reinterpret_cast<const jbyte *>(fv));
    env->CallVoidMethod(jobj,jmid_onRenderYUV,width,height,y,u,v);
    env->DeleteLocalRef(y);
    env->DeleteLocalRef(u);
    env->DeleteLocalRef(v);
    javaVM->DetachCurrentThread();

}

bool CallJava::isSupportCodec(const char *codec) {
    bool support = false;
    JNIEnv *env;
    if (javaVM->AttachCurrentThread(&env,0) != JNI_OK){
        LOGE("get child thread jnienv wrong");
        return support;
    }
    jstring type = env->NewStringUTF(codec);
    support = env->CallBooleanMethod(jobj, jmid_onSupportCodec, type);
    env->DeleteLocalRef(type);
    javaVM->DetachCurrentThread();
    return support;
}
