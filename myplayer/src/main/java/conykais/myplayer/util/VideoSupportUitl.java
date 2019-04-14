package conykais.myplayer.util;

import android.media.MediaCodecList;

import java.util.HashMap;
import java.util.Map;

public class VideoSupportUitl {

    private static Map<String, String> codecMap = new HashMap<>();

    static {
        codecMap.put("h264","video/avc");
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
                if(types[j].equals(findVideoCodecName(codecName)))
                {
                    support = true;
                    break;
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
