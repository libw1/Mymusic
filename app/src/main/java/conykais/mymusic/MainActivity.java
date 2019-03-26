package conykais.mymusic;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import conykais.myplayer.Demo;
import conykais.myplayer.TimeInfo;
import conykais.myplayer.TimeUtil;
import conykais.myplayer.listener.OnErrorListener;
import conykais.myplayer.listener.OnLoadListener;
import conykais.myplayer.listener.OnPauseResumeListener;
import conykais.myplayer.listener.OnPreparedListener;
import conykais.myplayer.listener.OnTimeInfoListenter;
import conykais.myplayer.player.Player;

public class MainActivity extends AppCompatActivity {

    private Player player;
    private TextView timeInfoText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        timeInfoText = findViewById(R.id.time_info);

        player = new Player();
        player.setPreparedListener(new OnPreparedListener() {
            @Override
            public void onPrepared() {
                Log.d("lbw","准备完成,开始解码");
                player.start();
            }
        });

        player.setOnLoadListener(new OnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                if (load){
                    Log.d("lbw", "加载中..");
                }else {
                    Log.d("lbw", "播放中..");
                }
            }
        });

        player.setPauseResumeListener(new OnPauseResumeListener() {
            @Override
            public void onPauseResume(boolean pause) {
                if (pause){
                    Log.d("lbw", "暂停中..");
                }else {
                    Log.d("lbw", "播放中..");
                }
            }
        });
        player.setTimeInfoListenter(new OnTimeInfoListenter() {
            @Override
            public void onTimeInfo(final TimeInfo timeInfo) {
                timeInfoText.post(new Runnable() {
                    @Override
                    public void run() {
                        timeInfoText.setText(TimeUtil.secdsToDateFormat(timeInfo.getDuration(),timeInfo.getDuration())
                                + "/" + TimeUtil.secdsToDateFormat(timeInfo.getCurrentTime(), timeInfo.getDuration()));
                    }
                });
            }
        });

        player.setErrorListener(new OnErrorListener() {
            @Override
            public void onError(int code, String msg) {
                Log.d("lbw", "onError: code = " + code + " msg = " + msg);
            }
        });
    }

    public void begin(View view) {
//        player.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        player.setSource("/sdcard/youtube-dl/lovestory.m4a");
//        player.setSource("http://ngcdn004.cnr.cn/live/dszs/index.m3u8");
//        player.setSource("http://192.168.0.124/htc.mp3");
//        player.setSource("/sdcard/youtube-dl/Red.m4a");
//        player.setSource("/sdcard/youtube-dl/you.mp4");
        player.prepare();
    }

    public void pause(View view) {
        player.pause();
    }

    public void resume(View view) {
        player.resume();
    }

    public void stop(View view) {
        player.stop();
    }
}
