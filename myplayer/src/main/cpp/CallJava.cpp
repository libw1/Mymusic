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
