#include <jni.h>
#include <string>
#include "FFmpeg.h"
#include "PlayStatus.h"

_JavaVM *javaVM = NULL;
CallJava *callJava = NULL;
FFmpeg *fFmpeg = NULL;
PlayStatus *playStatus = NULL;
pthread_t thread_start;


extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *res){
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **)(&env), JNI_VERSION_1_4) != JNI_OK){
        return result;
    }
    return JNI_VERSION_1_4;
}


extern "C"
JNIEXPORT void JNICALL
Java_conykais_myplayer_player_Player_n_1prepare(JNIEnv *env, jobject instance, jstring source_) {

    const char *source = env->GetStringUTFChars(source_,0);

    if (fFmpeg == NULL){
        if (callJava == NULL){
            callJava = new CallJava(javaVM,env,&instance);
        }
        playStatus = new PlayStatus();
        fFmpeg = new FFmpeg(playStatus,callJava,source);
    }
    fFmpeg->prepare();

}
void *startCallback(void *data){
    FFmpeg *fFmpeg = (FFmpeg *)(data);
    fFmpeg->start();
    pthread_exit(&thread_start);
}

extern "C"
JNIEXPORT void JNICALL
Java_conykais_myplayer_player_Player_n_1start(JNIEnv *env, jobject instance) {
    if (fFmpeg != NULL){
        pthread_create(&thread_start,NULL,startCallback,fFmpeg);

    }
}

extern "C"
JNIEXPORT void JNICALL
Java_conykais_myplayer_player_Player_n_1pause(JNIEnv *env, jobject instance) {

    if (fFmpeg != NULL){
        fFmpeg->pause();
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_conykais_myplayer_player_Player_n_1resume(JNIEnv *env, jobject instance) {
    if (fFmpeg != NULL){
        fFmpeg->resume();
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_conykais_myplayer_player_Player_n_1stop(JNIEnv *env, jobject instance) {

    if (fFmpeg != NULL){
        fFmpeg->release();
        delete (fFmpeg);
        fFmpeg = NULL;
        if (callJava != NULL){
            delete (callJava);
            callJava = NULL;
        }

        if (playStatus != NULL){
            delete(playStatus);
            playStatus = NULL;
        }
    }

}extern "C"
JNIEXPORT void JNICALL
Java_conykais_myplayer_player_Player_n_1seek(JNIEnv *env, jobject instance, jint secds) {

    if (fFmpeg != NULL){
        fFmpeg->seek(secds);
    }
}