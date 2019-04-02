package conykais.myplayer.player;

import android.text.TextUtils;
import android.util.Log;

import conykais.myplayer.MuteEnum;
import conykais.myplayer.TimeInfo;
import conykais.myplayer.listener.OnCompleteListener;
import conykais.myplayer.listener.OnErrorListener;
import conykais.myplayer.listener.OnLoadListener;
import conykais.myplayer.listener.OnPCMDBListener;
import conykais.myplayer.listener.OnPauseResumeListener;
import conykais.myplayer.listener.OnPreparedListener;
import conykais.myplayer.listener.OnTimeInfoListenter;

public class Player {

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
    }
    
    private String source;
    private OnPreparedListener preparedListener;
    private OnLoadListener onLoadListener;
    private OnPauseResumeListener pauseResumeListener;
    private OnTimeInfoListenter timeInfoListenter;
    private OnErrorListener errorListener;
    private OnCompleteListener completeListener;
    private OnPCMDBListener pcmdbListener;
    private static TimeInfo timeInfo;
    private static boolean playNext = false;
    private static int currentVolume = 100;
    private static int duration = -1;
    private static MuteEnum muteEnum = MuteEnum.MUTE_CENTER;
    private static float speed = 1.0f;
    private static float pitch = 1.0f;

    public static final String TAG = "lbw";
    
    public void setSource(String source){
        this.source = source;
    }

    public void setPreparedListener(OnPreparedListener preparedListener) {
        this.preparedListener = preparedListener;
    }

    public void setOnLoadListener(OnLoadListener onLoadListener) {
        this.onLoadListener = onLoadListener;
    }

    public void setPauseResumeListener(OnPauseResumeListener pauseResumeListener) {
        this.pauseResumeListener = pauseResumeListener;
    }

    public void setTimeInfoListenter(OnTimeInfoListenter timeInfoListenter) {
        this.timeInfoListenter = timeInfoListenter;
    }

    public void setErrorListener(OnErrorListener errorListener) {
        this.errorListener = errorListener;
    }

    public void setCompleteListener(OnCompleteListener completeListener) {
        this.completeListener = completeListener;
    }

    public void setPcmdbListener(OnPCMDBListener pcmdbListener) {
        this.pcmdbListener = pcmdbListener;
    }

    public void prepare(){
        if (TextUtils.isEmpty(source)){
            Log.d(TAG, "prepare: source not be empty");
            return;
        }
        callOnLoad(true);
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_prepare(source);
            }
        }).start();
    }

    public void start(){
        if (TextUtils.isEmpty(source)) {
            Log.d(TAG, "prepare: source not be empty");
            return;
        }

        new Thread(new Runnable() {
            @Override
            public void run() {
                setVolume(currentVolume);
                setMute(muteEnum);
                setSpeed(speed);
                setPitch(pitch);
                n_start();
            }
        }).start();
    }

    public void pause(){
        n_pause();
        if (pauseResumeListener != null){
            pauseResumeListener.onPauseResume(true);
        }
    }

    public void resume(){
        n_resume();
        if (pauseResumeListener != null){
            pauseResumeListener.onPauseResume(false);
        }
    }

    public void stop() {
        duration = -1;
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
            }
        }).start();
    }

    public void seek(int secds){
        n_seek(secds);
    }

    public void playNext(String url){
        source = url;
        playNext = true;
        stop();
    }

    public void setVolume(int volume){
        if (volume >= 0 && volume <= 100){
            currentVolume = volume;
            n_set_volume(volume);
        }
    }

    public int getCurrentVolume(){
        return currentVolume;
    }

    public int getDuration(){
        if (duration < 0){
            duration = n_duration();
        }
        return duration;
    }

    public void setMute(MuteEnum mute){
        muteEnum = mute;
        n_set_mute(mute.getValue());
    }

    public void setSpeed(float sp){
        speed = sp;
        n_set_speed(speed);
    }

    public void setPitch(float pit){
        pitch = pit;
        n_set_pitch(pit);
    }

    @SuppressWarnings("unused")
    public void onCallPrepare(){
        if (preparedListener != null){
            preparedListener.onPrepared();
        }
    }


    @SuppressWarnings("unused")
    private void callOnLoad(boolean load){
        if (onLoadListener != null){
            onLoadListener.onLoad(load);
        }
    }

    @SuppressWarnings("unused")
    public void onTimeInfo(int currentTime, int duration){
        if (timeInfoListenter != null){
            if (timeInfo == null){
                timeInfo = new TimeInfo();
            }
            timeInfo.setCurrentTime(currentTime);
            timeInfo.setDuration(duration);
            timeInfoListenter.onTimeInfo(timeInfo);
        }
    }

    @SuppressWarnings("unused")
    public void onError(int code , String msg){
        if (errorListener != null){
            stop();
            errorListener.onError(code,msg);
        }
    }

    @SuppressWarnings("unused")
    public void onComplete(){
        if (completeListener != null){
            stop();
            completeListener.onComplete();
        }
    }

    @SuppressWarnings("unused")
    public void onCallNext(){
        if (playNext){
            playNext = false;
            prepare();
        }
    }

    @SuppressWarnings("unused")
    public void onPCMDB(int db){
        if (pcmdbListener != null){
            pcmdbListener.onPCMDB(db);
        }
    }

    public native void n_prepare(String source);

    public native void n_start();

    public native void n_pause();

    public native void n_resume();

    public native void n_stop();

    public native void n_seek(int secds);

    public native void n_set_volume(int volume);

    public native int n_duration();

    public native void n_set_mute(int mute);

    public native void n_set_speed(float speed);

    public native void n_set_pitch(float pitch);
}
