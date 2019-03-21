package conykais.myplayer.player;

import android.text.TextUtils;
import android.util.Log;

import conykais.myplayer.listener.OnLoadListener;
import conykais.myplayer.listener.OnPauseResumeListener;
import conykais.myplayer.listener.OnPreparedListener;

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

    public void onCallPrepare(){
        if (preparedListener != null){
            preparedListener.onPrepared();
        }
    }

    private void callOnLoad(boolean load){
        if (onLoadListener != null){
            onLoadListener.onLoad(load);
        }
    }

    public native void n_prepare(String source);

    public native void n_start();

    public native void n_pause();

    public native void n_resume();
}
