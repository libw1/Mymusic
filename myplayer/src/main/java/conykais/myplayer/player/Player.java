package conykais.myplayer.player;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import conykais.myplayer.MuteEnum;
import conykais.myplayer.TimeInfo;
import conykais.myplayer.listener.OnCompleteListener;
import conykais.myplayer.listener.OnErrorListener;
import conykais.myplayer.listener.OnLoadListener;
import conykais.myplayer.listener.OnPCMDBListener;
import conykais.myplayer.listener.OnPauseResumeListener;
import conykais.myplayer.listener.OnPreparedListener;
import conykais.myplayer.listener.OnRecordTimeListener;
import conykais.myplayer.listener.OnTimeInfoListenter;
import conykais.myplayer.opengl.GLSurfaceView;
import conykais.myplayer.util.VideoSupportUitl;

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
    private OnRecordTimeListener recordTimeListener;
    private static TimeInfo timeInfo;
    private static boolean playNext = false;
    private static int currentVolume = 100;
    private static int duration = -1;
    private static MuteEnum muteEnum = MuteEnum.MUTE_CENTER;
    private static float speed = 1.0f;
    private static float pitch = 1.0f;
    private static boolean initMediaCodec = false;
    private GLSurfaceView glSurfaceView;

    private Surface surface;

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

    public void setRecordTimeListener(OnRecordTimeListener recordTimeListener) {
        this.recordTimeListener = recordTimeListener;
    }

    public void setGlSurfaceView(GLSurfaceView glSurfaceView) {
        this.glSurfaceView = glSurfaceView;
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
        timeInfo = null;
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

    public void startRecord(File file){
        if (!initMediaCodec){
            audioSampleRate = n_samplerate();
            if (audioSampleRate > 0){
                initMediaCodec = true;
                initMediaCodec(audioSampleRate,file);
                n_start_stop_record(true);
                Log.d(TAG, "startRecord: ");
            }
        }
    }

    public void stopRecord(){
        if (initMediaCodec){
            n_start_stop_record(false);
            releaseMedicacodec();
        }
    }

    public void pauseRecord(){
        n_start_stop_record(false);
        Log.d(TAG, "pauseRecord: ");
    }

    public void resumeRecord(){
        n_start_stop_record(true);
        Log.d(TAG, "resumeRecord: ");
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

    private native void n_prepare(String source);

    private native void n_start();

    private native void n_pause();

    private native void n_resume();

    private native void n_stop();

    private native void n_seek(int secds);

    private native void n_set_volume(int volume);

    private native int n_duration();

    private native void n_set_mute(int mute);

    private native void n_set_speed(float speed);

    private native void n_set_pitch(float pitch);

    private native int n_samplerate();

    private native void n_start_stop_record(boolean record);


    //MeidaCodec
    private MediaFormat mediaFormat;
    private MediaCodec encodec;
    private FileOutputStream outputStream;
    private MediaCodec.BufferInfo info;
    private int pcmdatasize;
    private byte[] outputBuffer;
    private int aacsamplerate = 4;
    private double recordTime;
    private int audioSampleRate;

    private void initMediaCodec(int sampleRate, File file){
        try {
            aacsamplerate = getADTSsamplerate(sampleRate);
            mediaFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC,sampleRate,2);
            mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE,96000);
            mediaFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
            mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 4096);
            encodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
            if (encodec == null){
                Log.d("lbw", "initMediaCodec: wrong");
                return;
            }
            recordTime = 0;
            info = new MediaCodec.BufferInfo();
            encodec.configure(mediaFormat,null,null,MediaCodec.CONFIGURE_FLAG_ENCODE);
            outputStream = new FileOutputStream(file);
            encodec.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void releaseMedicacodec() {

        if(encodec == null)
        {
            return;
        }
        try {
            outputStream.close();
            outputStream = null;
            encodec.stop();
            encodec.release();
            encodec = null;
            mediaFormat = null;
            info = null;
            initMediaCodec = false;
            recordTime = 0;
            Log.d(TAG, "录制完成...");
        } catch (IOException e) {
            e.printStackTrace();
        }
        finally {
            if(outputStream != null)
            {
                try {
                    outputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                outputStream = null;
            }
        }
    }

    private int getADTSsamplerate(int sampleRate) {
        int rate = 4;
        switch (sampleRate)
        {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }


    @SuppressWarnings("unused")
    private void encodecPcmToAcc(int size, byte[] buffer){
        if(buffer != null && encodec != null) {
            recordTime += size * 1.0/ (audioSampleRate * 2 * (16 / 8));
            if (recordTimeListener != null){
                recordTimeListener.onRecordTime((int) recordTime);
            }
            int inputBufferindex = encodec.dequeueInputBuffer(0);
            if(inputBufferindex >= 0) {
                ByteBuffer byteBuffer = encodec.getInputBuffers()[inputBufferindex];
                byteBuffer.clear();
                byteBuffer.put(buffer);
                encodec.queueInputBuffer(inputBufferindex, 0, size, 0, 0);
            }

            int index = encodec.dequeueOutputBuffer(info, 0);
            while(index >= 0) {
                try {
                    pcmdatasize = info.size + 7;
                    outputBuffer = new byte[pcmdatasize];

                    ByteBuffer byteBuffer = encodec.getOutputBuffers()[index];
                    byteBuffer.position(info.offset);
                    byteBuffer.limit(info.offset + info.size);

                    addADtsHeader(outputBuffer, pcmdatasize, aacsamplerate);

                    byteBuffer.get(outputBuffer, 7, info.size);
                    byteBuffer.position(info.offset);
                    outputStream.write(outputBuffer, 0, pcmdatasize);

                    encodec.releaseOutputBuffer(index, false);
                    index = encodec.dequeueOutputBuffer(info, 0);
                    outputBuffer = null;
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void addADtsHeader(byte[] packet, int packetLen, int samplerate) {

        int profile = 2; // AAC LC
        int freqIdx = samplerate; // samplerate
        int chanCfg = 2; // CPE

        packet[0] = (byte) 0xFF; // 0xFFF(12bit) 这里只取了8位，所以还差4位放到下一个里面
        packet[1] = (byte) 0xF9; // 第一个t位放F
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }

    @SuppressWarnings("unused")
    private void onCallRenderYUV(int width,int height, byte[] y, byte[] u,byte[] v){
//        Log.d("lbw", "onCallRenderYUV: ");
        if (glSurfaceView != null){
            glSurfaceView.setYUVData(width,height,y,u,v);
        }
    }

    private boolean onCallSupportMediaCodec(String codec){
        Log.d(TAG, "onCallSupportMediaCodec: ");
        return VideoSupportUitl.isSupportCodec(codec);
    }

    /**
     * 初始化MediaCodec
     * @param codecName
     * @param width
     * @param height
     * @param csd_0
     * @param csd_1
     */
    public void initMediaCodec(String codecName, int width, int height, byte[] csd_0, byte[] csd_1)
    {
        if(surface != null)
        {
            try {
                String mime = VideoSupportUitl.findVideoCodecName(codecName);
                mediaFormat = MediaFormat.createVideoFormat(mime, width, height);
                mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, width * height);
                mediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(csd_0));
                mediaFormat.setByteBuffer("csd-1", ByteBuffer.wrap(csd_1));
                Log.d("lbw",mediaFormat.toString());
                encodec = MediaCodec.createDecoderByType(mime);

                encodec.configure(mediaFormat, surface, null, 0);
                encodec.start();

            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
        }
        else
        {
            if(errorListener != null)
            {
                errorListener.onError(2001, "surface is null");
            }
        }
    }


    public void decodeAVPacket(int datasize, byte[] data)
    {
        if(surface != null && datasize > 0 && data != null)
        {
            int intputBufferIndex = encodec.dequeueInputBuffer(10);
            if(intputBufferIndex >= 0)
            {
                ByteBuffer byteBuffer = encodec.getOutputBuffers()[intputBufferIndex];
                byteBuffer.clear();
                byteBuffer.put(data);
                encodec.queueInputBuffer(intputBufferIndex, 0, datasize, 0, 0);
            }
            int outputBufferIndex = encodec.dequeueOutputBuffer(info, 10);
            while(outputBufferIndex >= 0)
            {
                encodec.releaseOutputBuffer(outputBufferIndex, true);
                outputBufferIndex = encodec.dequeueOutputBuffer(info, 10);
            }
        }
    }
}
