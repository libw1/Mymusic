package conykais.myplayer.opengl;

import android.content.Context;
import android.util.AttributeSet;

public class GLSurfaceView extends android.opengl.GLSurfaceView{

    private conykais.myplayer.opengl.Renderer renderer;
    public GLSurfaceView(Context context) {
        this(context,null);
    }

    public GLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        renderer = new conykais.myplayer.opengl.Renderer(context);
        setRenderer(renderer);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void setYUVData(int width, int height, byte[] y, byte[] u, byte[] v){

        if (renderer != null){
            renderer.setYUVRendererData(width,height,y,u,v);
            requestRender();
        }
    }


}
