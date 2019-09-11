package dev.flutter.scenarios;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.util.HashMap;

import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.PluginRegistry;

public class TestExternalTexture {
    private volatile Handler mGLHandler;
    private volatile Handler mMainHandler;
    private final FlutterEngine flutterEngine;
    private int imageTexture;
    /**
     * Plugin registration.
     */
    @SuppressLint("UseSparseArrays")
    public TestExternalTexture(FlutterEngine engine) {
        this.flutterEngine = engine;
        startThread();
    }

    public void createTexture(TextureComplete complete){
        if(mGLHandler != null){
            mGLHandler.post(()->{
                final int[] textures = new int[1];
                GLES20.glGenTextures(1, textures, 0);
                imageTexture = textures[0];
                if(mMainHandler == null){
                    mMainHandler = new Handler();
                }
                mMainHandler.post(()->{
                    complete.onTextureCreated(imageTexture);
                });
            });
        }
        else{
            if(mMainHandler == null){
                mMainHandler = new Handler();
            }
            mMainHandler.postDelayed(()->{
                this.createTexture(complete);
            },200);
        }
    }

    void startWithId(long textureIndex, Context context){
        mGLHandler.post(()->{

            int identifier = context.getResources().getIdentifier("flutter","mipmap",context.getPackageName());

            final Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(),identifier);

            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, imageTexture);

            GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER,
                    GLES20.GL_NEAREST);
            GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER,
                    GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S,
                    GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T,
                    GLES20.GL_CLAMP_TO_EDGE);
            try {
                GLUtils.texImage2D(GLES20.GL_TEXTURE_2D,0,bitmap,0);
            }
            catch (Exception e){
            }

            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);

            mMainHandler.post(() -> flutterEngine.getRenderer().onShareFrameAvaliable(textureIndex));
        });
    }


    private void startThread(){
        Thread glThread = new Thread(new Runnable() {
            @Override
            public void run() {
                Looper.prepare();
                mGLHandler = new Handler();
                initOpenGL();
                Looper.loop();
            }
        });
        glThread.setName("external.testqueue");
        glThread.start();
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    private void initOpenGL(){
        EGLContext sharedContext = flutterEngine.getRenderer().getShareContext(Build.VERSION.SDK_INT);
        EGLDisplay eglDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
        if (eglDisplay == EGL14.EGL_NO_DISPLAY) {
            throw new RuntimeException("unable to get EGL14 display");
        }
        int[] version = new int[2];
        if (!EGL14.eglInitialize(eglDisplay, version, 0, version, 1)) {
            throw new RuntimeException("unable to initialize EGL14");
        }
        EGLConfig config = null;
        config = TestExternalTexture.getConfig2(eglDisplay);
        if (config == null) {
            throw new RuntimeException("Unable to find a suitable EGLConfig");
        }
        int[] attrib2_list = {
                EGL14.EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL14.EGL_NONE
        };
        EGLContext myEGLContext = EGL14.eglCreateContext(eglDisplay, config, sharedContext,
                attrib2_list, 0);

        // Confirm with query.
        int[] values = new int[1];
        EGL14.eglQueryContext(eglDisplay, myEGLContext, EGL14.EGL_CONTEXT_CLIENT_VERSION,
                values, 0);

        int[] surfaceAttribs = {
                EGL14.EGL_WIDTH, 16,
                EGL14.EGL_HEIGHT, 16,
                EGL14.EGL_NONE
        };
        EGLSurface eglSurface = EGL14.eglCreatePbufferSurface(eglDisplay, config,
                surfaceAttribs, 0);
        EGL14.eglMakeCurrent(eglDisplay,eglSurface,eglSurface,myEGLContext);
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    static EGLConfig getConfig2(EGLDisplay display) {

        // The actual surface is generally RGBA or RGBX, so situationally omitting alpha
        // doesn't really help.  It can also lead to a huge performance hit on glReadPixels()
        // when reading into a GL_RGBA buffer.
        int[] attribList = {
                EGL14.EGL_RED_SIZE, 8,
                EGL14.EGL_GREEN_SIZE, 8,
                EGL14.EGL_BLUE_SIZE, 8,
                EGL14.EGL_ALPHA_SIZE, 8,
                //EGL14.EGL_DEPTH_SIZE, 16,
                //EGL14.EGL_STENCIL_SIZE, 8,
                EGL14.EGL_RENDERABLE_TYPE,EGL14.EGL_OPENGL_ES2_BIT,
                EGL14.EGL_NONE
        };

        EGLConfig[] configs = new EGLConfig[1];
        int[] numConfigs = new int[1];
        if (!EGL14.eglChooseConfig(display, attribList, 0, configs, 0, configs.length,
                numConfigs, 0)) {
            return null;
        }
        return configs[0];
    }

}
