package conykais.myplayer.opengl;

import android.content.Context;
import android.opengl.GLES20;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class ShaderUtil {

    public static String readRawText(Context context, int rawId){
        InputStream inputStream = context.getResources().openRawResource(rawId);
        BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
        StringBuilder sb = new StringBuilder();
        String line = null;
        try {
            while ((line = reader.readLine()) != null){
                sb.append(line).append("\n");
            }
            reader.close();
        }catch (IOException e){
            e.printStackTrace();
        }
        return sb.toString();
    }


    public static int createProgress(String vertexSource, String fragmentSource) {
        int vertex = loadShader(GLES20.GL_VERTEX_SHADER,vertexSource);
        if (vertex == 0){
            return 0;
        }
        int fragment = loadShader(GLES20.GL_FRAGMENT_SHADER,fragmentSource);
        if (fragment == 0){
            return 0;
        }

        int program = GLES20.glCreateProgram();
        if (program != 0){
            GLES20.glAttachShader(program,vertex);
            GLES20.glAttachShader(program,fragment);
            GLES20.glLinkProgram(program);
            int[] linkStatus = new int[1];
            GLES20.glGetProgramiv(program,GLES20.GL_LINK_STATUS,linkStatus,0);
            if (linkStatus[0] != GLES20.GL_TRUE){
                GLES20.glDeleteProgram(program);
                program = 0;
            }
        }
        return program;
    }

    private static int loadShader(int shaderType, String shaderSource) {
        int shader = GLES20.glCreateShader(shaderType);
        if (shader != 0){
            GLES20.glShaderSource(shader,shaderSource);
            GLES20.glCompileShader(shader);
            int[] compile = new int[1];
            GLES20.glGetShaderiv(shader,GLES20.GL_COMPILE_STATUS,compile,0);
            if (compile[0] != GLES20.GL_TRUE){
                Log.d("lbw", "loadShader: error");
                GLES20.glDeleteShader(shader);
                shader = 0;
            }
        }
        return shader;
    }
}
