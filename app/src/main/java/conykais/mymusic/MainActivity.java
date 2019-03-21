package conykais.mymusic;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import conykais.myplayer.Demo;
import conykais.myplayer.listener.OnLoadListener;
import conykais.myplayer.listener.OnPauseResumeListener;
import conykais.myplayer.listener.OnPreparedListener;
import conykais.myplayer.player.Player;

public class MainActivity extends AppCompatActivity {

    private Player player;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

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
    }

    public void begin(View view) {
//        player.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        player.setSource("/sdcard/youtube-dl/lovestroy.m4a");
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
}
