package conykais.myplayer.util;

import android.media.MediaCodecList;
import android.util.Log;

import java.util.HashMap;
import java.util.Map;

public class VideoSupportUitl {

    private static Map<String, String> codecMap = new HashMap<>();

    static {
        codecMap.put("h264","video/avc");
        codecMap.put("mpeg4","video/mp4v-es");
        codecMap.put("vp9","video/x-vnd.on2.vp9");
        codecMap.put("h265","video/hevc");
        codecMap.put("vp8","video/x-vnd.on2.vp8");
        codecMap.put("h263","video/3gpp");
    }

    public static String findVideoCodecName(String codec){
        if (codecMap.containsKey(codec)){
            return codecMap.get(codec);
        }
        return "";
    }

    public static boolean isSupportCodec(String codecName){
        boolean support = false;
        int count = MediaCodecList.getCodecCount();
        for (int i = 0; i < count; i++){
            String[] types = MediaCodecList.getCodecInfoAt(i).getSupportedTypes();
            for(int j = 0; j < types.length; j++)
            {
                Log.d("lbw", "isSupportCodec: " + types[j]);
                if(types[j].equals(findVideoCodecName(codecName)))
                {
                    support = true;
//                    break;
                }
            }
            if(support)
            {
                break;
            }
        }
        return support;
    }
}
