#include <jni.h>
#include <string>
#include "FFmpeg.h"
#include "PlayStatus.h"

_JavaVM *javaVM = NULL;
CallJava *callJava = NULL;
FFmpeg *fFmpeg = NULL;
PlayStatus *playStatus = NULL;
pthread_t thread_start;
bool nexit = true;

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

    if (!nexit){
        return;
    }
    jclass jcz = env->GetObjectClass(instance);
    jmethodID jmid_next = env->GetMethodID(jcz,"onCallNext","()V");
    nexit = false;
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
    nexit = true;
    env->CallVoidMethod(instance,jmid_next);

}

extern "C"
JNIEXPORT void JNICALL
Java_conykais_myplayer_player_Player_n_1seek(JNIEnv *env, jobject instance, jint secds) {

    if (fFmpeg != NULL){
        fFmpeg->seek(secds);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_conykais_myplayer_player_Player_n_1set_1volume(JNIEnv *env, jobject instance, jint volume) {

    if (fFmpeg != NULL){
        fFmpeg->setVolume(volume);
    }

}

extern "C"
JNIEXPORT jint JNICALL
Java_conykais_myplayer_player_Player_n_1duration(JNIEnv *env, jobject instance) {

    if (fFmpeg != NULL){
        return fFmpeg->duration;
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_conykais_myplayer_player_Player_n_1set_1mute(JNIEnv *env, jobject instance, jint mute) {

    if (fFmpeg != NULL){
        fFmpeg->setMute(mute);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_conykais_myplayer_player_Player_n_1set_1speed(JNIEnv *env, jobject instance, jfloat speed) {

    if (fFmpeg != NULL){
        fFmpeg->setSpeed(speed);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_conykais_myplayer_player_Player_n_1set_1pitch(JNIEnv *env, jobject instance, jfloat pitch) {
    if (fFmpeg != NULL){
        fFmpeg->setPitch(pitch);
    }

}extern "C"
JNIEXPORT jint JNICALL
Java_conykais_myplayer_player_Player_n_1samplerate(JNIEnv *env, jobject instance) {

    if (fFmpeg != NULL){
        return fFmpeg->getSampleRate();
    }
    return 0;
}