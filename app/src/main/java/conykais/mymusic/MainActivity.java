package conykais.mymusic;

import android.content.pm.ActivityInfo;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import java.io.File;

import conykais.myplayer.MuteEnum;
import conykais.myplayer.TimeInfo;
import conykais.myplayer.TimeUtil;
import conykais.myplayer.listener.OnCompleteListener;
import conykais.myplayer.listener.OnErrorListener;
import conykais.myplayer.listener.OnLoadListener;
import conykais.myplayer.listener.OnPCMDBListener;
import conykais.myplayer.listener.OnPauseResumeListener;
import conykais.myplayer.listener.OnPreparedListener;
import conykais.myplayer.listener.OnRecordTimeListener;
import conykais.myplayer.listener.OnTimeInfoListenter;
import conykais.myplayer.opengl.GLSurfaceView;
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
    private GLSurfaceView glSurfaceView;
    private boolean isFull = false;
    private LinearLayout ll_othre;
    private RelativeLayout rl_title;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        StatusBar statusBar = new StatusBar(MainActivity.this);
        //设置颜色为半透明
//        statusBar.setColor(R.color.translucent);
//        //设置颜色为透明
        statusBar.setColor(R.color.transparent);
//        //隐藏状态栏
//        statusBar.hide();
        statusBar.setTextColor(false);
        setContentView(R.layout.activity_main);

        timeInfoText = findViewById(R.id.time_info);
        voiceText = findViewById(R.id.voice_info);
        playSeekBar = findViewById(R.id.play_seek_bar);
        voiceSeekbar = findViewById(R.id.voice_seek_bar);
        glSurfaceView = findViewById(R.id.gl_surface_view);
        ll_othre = findViewById(R.id.ll_other);
        rl_title = findViewById(R.id.rl_title);

        player = new Player();
        player.setVolume(50);
//        player.setMute(MuteEnum.MUTE_RIGHT);
        voiceText.setText("音量：" + player.getCurrentVolume() + "%");
        voiceSeekbar.setProgress(player.getCurrentVolume());
        player.setGlSurfaceView(glSurfaceView);

//        player.setSpeed(1.5f);
//        player.setPitch(1.5f);

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

        player.setPcmdbListener(new OnPCMDBListener() {
            @Override
            public void onPCMDB(int db) {
//                Log.d("lbw", "pcm db is " + db);
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

        player.setRecordTimeListener(new OnRecordTimeListener() {
            @Override
            public void onRecordTime(int time) {
                Log.d("lbw", "onRecordTime: " + time);
            }
        });

    }

    public void begin(View view) {
//        player.setSource("https://xinhudev.simu800.com/xh/sign-manager/files/video/20220706/d49d04ad0aee47d294b29bcefe25f6a2.mp4");
//        player.setSource("/sdcard/youtube-dl/lovestory.m4a");
//        player.setSource("/sdcard/youtube-dl/22.m4a");
//        player.setSource("http://ngcdn004.cnr.cn/live/dszs/index.m3u8");
//        player.setSource("http://ngcdn004.cnr.cn/live/dszs/index.m3u8");
//        player.setSource("http://192.168.0.124/htc.mp3");
//        player.setSource("/sdcard/meix/2021-09-09-11-15-52.mp4");
//        player.setSource("/sdcard/DCIM/Camera/VID_20210929_165136_8K.mp4");
//        player.setSource("/sdcard/DCIM/Camera/VID_20230308_175338.mp4");
//        player.setSource("/sdcard/DCIM/Camera/VID_20210915_102200.mp4");
//        player.setSource("/sdcard/video.mp4");
                player.setSource("/sdcard/Movies/英雄联盟手游精彩时刻/20220519010057_0.mp4");
//        player.setSource("/sdcard/youtube-dl/waypoint.mp4");
//        player.setSource("/sdcard/youtube-dl/1080sh.mp4");
//        player.setSource("/sdcard/youtube-dl/测试.h265");
//        player.setSource("/sdcard/youtube-dl/cq.mp4");
//        player.setSource("/sdcard/youtube-dl/bdb.ape");
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
//        player.playNext("/sdcard/youtube-dl/Red.m4a");
//        player.playNext("http://ngcdn004.cnr.cn/live/dszs/index.m3u8");
//        player.playNext("http://ngcdn004.cnr.cn/live/dszs/index12.m3u8");
//        player.playNext("http://ngcdn004.cnr.cn/live/dszs/index.m3u8");
        player.playNext("/sdcard/youtube-dl/you.mp4");
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

    public void record(View view) {
        player.startRecord(new File("/sdcard/youtube-dl/record.aac"));
    }

    public void stopRecord(View view) {
        player.stopRecord();
    }

    public void resumeRecord(View view) {
        player.resumeRecord();
    }

    public void pauseRecord(View view) {
        player.pauseRecord();
    }

    public void full(View view) {
        if (isFull) {
            isFull = false;
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
            ll_othre.setVisibility(View.VISIBLE);
            rl_title.setVisibility(View.VISIBLE);
            LinearLayout.LayoutParams params = (LinearLayout.LayoutParams) glSurfaceView.getLayoutParams();
            params.height = 555;
            glSurfaceView.setLayoutParams(params);
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN); //显示状态栏
        } else {
            isFull = true;
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
            ll_othre.setVisibility(View.GONE);
            rl_title.setVisibility(View.GONE);
            LinearLayout.LayoutParams params = (LinearLayout.LayoutParams) glSurfaceView.getLayoutParams();
            params.height = LinearLayout.LayoutParams.MATCH_PARENT;
            glSurfaceView.setLayoutParams(params);
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);//隐藏状态栏
        }
    }

    @Override
    public void onBackPressed() {
        if (isFull) {
            full(glSurfaceView);
            return;
        }
        super.onBackPressed();
    }
}
