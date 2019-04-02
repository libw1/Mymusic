package conykais.mymusic;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import conykais.myplayer.Demo;
import conykais.myplayer.MuteEnum;
import conykais.myplayer.TimeInfo;
import conykais.myplayer.TimeUtil;
import conykais.myplayer.listener.OnCompleteListener;
import conykais.myplayer.listener.OnErrorListener;
import conykais.myplayer.listener.OnLoadListener;
import conykais.myplayer.listener.OnPauseResumeListener;
import conykais.myplayer.listener.OnPreparedListener;
import conykais.myplayer.listener.OnTimeInfoListenter;
import conykais.myplayer.player.Player;

public class MainActivity extends AppCompatActivity {

    private Player player;
    private TextView timeInfoText;
    private int time;
    private TextView voiceText;
    private SeekBar playSeekBar;
    private SeekBar voiceSeekbar;
    private boolean isSeekBar;
    private int position = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        timeInfoText = findViewById(R.id.time_info);
        voiceText = findViewById(R.id.voice_info);
        playSeekBar = findViewById(R.id.play_seek_bar);
        voiceSeekbar = findViewById(R.id.voice_seek_bar);

        player = new Player();
        player.setVolume(50);
        player.setMute(MuteEnum.MUTE_RIGHT);
        voiceText.setText("音量：" + player.getCurrentVolume() + "%");
        voiceSeekbar.setProgress(player.getCurrentVolume());

        player.setSpeed(1.5f);
        player.setPitch(1.5f);

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
                time = timeInfo.getCurrentTime();
                timeInfoText.post(new Runnable() {
                    @Override
                    public void run() {
                        if (!isSeekBar) {
                            timeInfoText.setText(TimeUtil.secdsToDateFormat(timeInfo.getDuration(), timeInfo.getDuration())
                                    + "/" + TimeUtil.secdsToDateFormat(timeInfo.getCurrentTime(), timeInfo.getDuration()));
                            playSeekBar.setProgress(timeInfo.getCurrentTime() * 100 / timeInfo.getDuration());
                        }
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

        player.setCompleteListener(new OnCompleteListener() {
            @Override
            public void onComplete() {
                Log.d("lbw", "播放完成!");
            }
        });

        voiceSeekbar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                player.setVolume(progress);
                voiceText.setText("音量：" + player.getCurrentVolume() + "%");
                Log.d("lbw", "progress is " + progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        playSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (player.getDuration() > 0 && isSeekBar){
                    position = progress * player.getDuration() / 100;
                    timeInfoText.setText(TimeUtil.secdsToDateFormat(player.getDuration(), player.getDuration())
                            + "/" + TimeUtil.secdsToDateFormat(progress * player.getDuration() / 100 , player.getDuration()));
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isSeekBar = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                player.seek(position);
                isSeekBar = false;
            }
        });

    }

    public void begin(View view) {
//        player.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        player.setSource("/sdcard/youtube-dl/lovestory.m4a");
//        player.setSource("/sdcard/youtube-dl/22.m4a");
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

    public void next(View view) {
        player.playNext("/sdcard/youtube-dl/Red.m4a");
    }

    public void right(View view) {
        player.setMute(MuteEnum.MUTE_RIGHT);
    }

    public void left(View view) {
        player.setMute(MuteEnum.MUTE_LEFT);
    }

    public void center(View view) {
        player.setMute(MuteEnum.MUTE_CENTER);
    }

    public void speed(View view) {
        player.setSpeed(1.5f);
        player.setPitch(1.0f);
    }

    public void pitch(View view) {
        player.setSpeed(1.0f);
        player.setPitch(1.5f);
    }

    public void changeall(View view) {
        player.setSpeed(1.5f);
        player.setPitch(1.5f);
    }

    public void normal(View view) {
        player.setSpeed(1.0f);
        player.setPitch(1.0f);
    }
}
