package conykais.mymusic;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import conykais.myplayer.Demo;
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
    }

    public void begin(View view) {
//        player.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
//        player.setSource("/sdcard/youtube-dl/lovestroy.m4a");
//        player.setSource("/sdcard/youtube-dl/Red.m4a");
        player.setSource("/sdcard/youtube-dl/you.mp4");
        player.prepare();
    }
}
