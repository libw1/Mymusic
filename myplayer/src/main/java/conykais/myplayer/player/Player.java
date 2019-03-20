package conykais.myplayer.player;

import android.text.TextUtils;
import android.util.Log;

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
    public OnPreparedListener preparedListener;
    public static final String TAG = "lbw";
    
    public void setSource(String source){
        this.source = source;
    }

    public void setPreparedListener(OnPreparedListener preparedListener) {
        this.preparedListener = preparedListener;
    }
    
    public void prepare(){
        if (TextUtils.isEmpty(source)){
            Log.d(TAG, "prepare: source not be empty");
            return;
        }
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

    public void onCallPrepare(){
        if (preparedListener != null){
            preparedListener.onPrepared();
        }
    }

    public native void n_prepare(String source);

    public native void n_start();
}
