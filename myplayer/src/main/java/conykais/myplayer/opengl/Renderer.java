package conykais.myplayer.opengl;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.view.Surface;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import conykais.myplayer.R;
import conykais.myplayer.util.ShaderUtil;

public class Renderer implements GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener {


    public static final int RENDER_YUV = 1;
    public static final int RENDER_MEDIACODEC = 2;
    private Context context;

    private final float[] vertexData ={

            -1f, -1f,
            1f, -1f,
            -1f, 1f,
            1f, 1f

    };

    private final float[] textureData ={
            0f,1f,
            1f, 1f,
            0f, 0f,
            1f, 0f
    };

    private int renderType = RENDER_YUV;
    private FloatBuffer vertexBuffer;
    private FloatBuffer textureBuffer;
    private int program_yuv;
    private int avPosition_yuv;
    private int afPosition_yuv;

    private int sample_y;
    private int sample_u;
    private int sample_v;
    private int[] textureId_yuv;

    private int width_yuv;
    private int height_yuv;
    private ByteBuffer y;
    private ByteBuffer u;
    private ByteBuffer v;

    //mediacodec
    private int program_mediacodec;
    private int avPosition_mediacodec;
    private int afPosition_mediacodec;
    private int samplerOES_mediacodec;
    private int textureId_mediacodec;

    private float[] mMatrix;
    private int mVertexMatrixHandler;//矩阵变化接受者

    private int mWorldWidth = -1;
    private int mWorldHeight = -1;
    private int mVideoWidth = -1;
    private int mVideoHeight = -1;

    private SurfaceTexture surfaceTexture;
    private Surface surface;

    private OnSurfaceCreateListener onSurfaceCreateListener;
    private OnRenderListener onRenderListener;
    public Renderer(Context context){
        this.context = context;
        vertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer().put(vertexData);
        vertexBuffer.position(0);

        textureBuffer = ByteBuffer.allocateDirect(textureData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer().put(textureData);
        textureBuffer.position(0);

    }

    public void setVideoHeight(int mVideoHeight) {
        this.mVideoHeight = mVideoHeight;
    }

    public void setVideoWidth(int mVideoWidth) {
        this.mVideoWidth = mVideoWidth;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        initRenderYUV();
        initRenderMediacodec();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0,0,width,height);
        this.mWorldWidth = width;
        this.mWorldHeight = height;
    }

    public void setOnSurfaceCreateListener(OnSurfaceCreateListener onSurfaceCreateListener) {
        this.onSurfaceCreateListener = onSurfaceCreateListener;
    }

    public void setOnRenderListener(OnRenderListener onRenderListener) {
        this.onRenderListener = onRenderListener;
    }

    public void setRenderType(int renderType) {
        this.renderType = renderType;
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(0.0f,0.0f,0.0f,1.0f);
        if(renderType == RENDER_YUV){
            renderYUV();
        }else if(renderType == RENDER_MEDIACODEC) {
            renderMediacodec();
        }
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    }

    private void initRenderYUV(){
        String vertexSource = ShaderUtil.readRawText(context, R.raw.vertex_shader);
        String fragmentSource = ShaderUtil.readRawText(context,R.raw.fragment_yuv);
        program_yuv = ShaderUtil.createProgress(vertexSource,fragmentSource);

        avPosition_yuv = GLES20.glGetAttribLocation(program_yuv,"av_Position");
        afPosition_yuv = GLES20.glGetAttribLocation(program_yuv,"af_Position");
        mVertexMatrixHandler = GLES20.glGetUniformLocation(program_mediacodec, "uMatrix");

        sample_y = GLES20.glGetUniformLocation(program_yuv,"sampler_y");
        sample_u = GLES20.glGetUniformLocation(program_yuv,"sampler_u");
        sample_v = GLES20.glGetUniformLocation(program_yuv,"sampler_v");

        textureId_yuv = new int[3];
        GLES20.glGenTextures(3,textureId_yuv,0);

        for (int i = 0; i < 3; i++){
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[i]);

            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        }
    }

    public void setYUVRendererData(int width, int height, byte[] y,byte[] u,byte[]v){
        this.width_yuv = width;
        this.height_yuv = height;
        setVideoWidth(width);
        setVideoHeight(height);
        this.y = ByteBuffer.wrap(y);
        this.u = ByteBuffer.wrap(u);
        this.v = ByteBuffer.wrap(v);
    }

    private void renderYUV(){
        if (width_yuv > 0 && height_yuv > 0 && y != null && u != null && v != null) {

            initDefMatrix();
            GLES20.glUseProgram(program_yuv);
            GLES20.glEnableVertexAttribArray(avPosition_yuv);
            GLES20.glVertexAttribPointer(avPosition_yuv, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer);
//        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP,0,4);

            GLES20.glEnableVertexAttribArray(afPosition_yuv);
            GLES20.glVertexAttribPointer(afPosition_yuv, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[0]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv, height_yuv, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, y);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[1]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv / 2, height_yuv / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, u);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[2]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv / 2, height_yuv / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, v);

            GLES20.glUniform1i(sample_y, 0);
            GLES20.glUniform1i(sample_u, 1);
            GLES20.glUniform1i(sample_v, 2);

            GLES20.glUniformMatrix4fv(mVertexMatrixHandler, 1, false, mMatrix, 0);

            y.clear();
            u.clear();
            v.clear();
            y = null;
            u = null;
            v = null;

        }
    }

    private void initRenderMediacodec() {
        String vertexSource = ShaderUtil.readRawText(context, R.raw.vertex_shader);
        String fragmentSource = ShaderUtil.readRawText(context, R.raw.fragment_mediacodec);
        program_mediacodec = ShaderUtil.createProgress(vertexSource, fragmentSource);

        avPosition_mediacodec = GLES20.glGetAttribLocation(program_mediacodec, "av_Position");
        afPosition_mediacodec = GLES20.glGetAttribLocation(program_mediacodec, "af_Position");
        samplerOES_mediacodec = GLES20.glGetUniformLocation(program_mediacodec, "sTexture");
        mVertexMatrixHandler = GLES20.glGetUniformLocation(program_mediacodec, "uMatrix");

        int[] textureids = new int[1];
        GLES20.glGenTextures(1, textureids, 0);
        textureId_mediacodec = textureids[0];

        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);

        surfaceTexture = new SurfaceTexture(textureId_mediacodec);
        surface = new Surface(surfaceTexture);
        surfaceTexture.setOnFrameAvailableListener(this);

        if(onSurfaceCreateListener != null)
        {
            onSurfaceCreateListener.onSurfaceCreate(surface);
        }
    }

    private void renderMediacodec(){
        initDefMatrix();
        GLES20.glUseProgram(program_mediacodec);

        GLES20.glEnableVertexAttribArray(avPosition_mediacodec);
        GLES20.glVertexAttribPointer(avPosition_mediacodec, 2, GLES20.GL_FLOAT, false, 0, vertexBuffer);

        GLES20.glEnableVertexAttribArray(afPosition_mediacodec);
        GLES20.glVertexAttribPointer(afPosition_mediacodec, 2, GLES20.GL_FLOAT, false, 0, textureBuffer);

        GLES20.glUniformMatrix4fv(mVertexMatrixHandler, 1, false, mMatrix, 0);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId_mediacodec);
        GLES20.glUniform1i(samplerOES_mediacodec, 0);
        surfaceTexture.updateTexImage();

    }

    private void initDefMatrix() {
        if (mVideoWidth != -1 && mVideoHeight != -1 &&
                mWorldWidth != -1 && mWorldHeight != -1) {
            mMatrix = new float[16];
            float originRatio = mVideoWidth / (float)mVideoHeight;
            float worldRatio = mWorldWidth / (float)mWorldHeight;
            if (mWorldWidth > mWorldHeight) {
                if (originRatio > worldRatio) {
                    float actualRatio = originRatio / worldRatio;
                    Matrix.orthoM(
                            mMatrix, 0,
                            -1f, 1f,
                            -actualRatio, actualRatio,
                            -1f, 3f
                    );
                } else {// 原始比例小于窗口比例，缩放高度度会导致高度超出，因此，高度以窗口为准，缩放宽度
                    float actualRatio = worldRatio / originRatio;
                    Matrix.orthoM(
                            mMatrix, 0,
                            -actualRatio, actualRatio,
                            -1f, 1f,
                            -1f, 3f
                    );
                }
            } else {
                if (originRatio > worldRatio) {
                    float actualRatio = originRatio / worldRatio;
                    Matrix.orthoM(
                            mMatrix, 0,
                            -1f, 1f,
                            -actualRatio, actualRatio,
                            -1f, 3f
                    );
                } else {// 原始比例小于窗口比例，缩放高度会导致高度超出，因此，高度以窗口为准，缩放宽度
                    float actualRatio = worldRatio / originRatio;
                    Matrix.orthoM(
                            mMatrix, 0,
                            -actualRatio, actualRatio,
                            -1f, 1f,
                            -1f, 3f
                    );
                }
            }
        }
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        if(onRenderListener != null){
            onRenderListener.onRender();
        }
    }


    public interface OnSurfaceCreateListener {
        void onSurfaceCreate(Surface surface);
    }

    public interface OnRenderListener{
        void onRender();
    }
}
