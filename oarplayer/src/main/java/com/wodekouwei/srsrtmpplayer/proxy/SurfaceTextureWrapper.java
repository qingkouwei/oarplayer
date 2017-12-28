package com.wodekouwei.srsrtmpplayer.proxy;

import android.graphics.SurfaceTexture;
import android.util.Log;
import android.view.Surface;

public class SurfaceTextureWrapper {
    private final static String TAG = "SurfaceTextureWrapper";
    private static SurfaceTexture texture;
    private static Surface surface;
    private static float[] matrix = new float[16];

    public static Surface getSurface(int name){
        Log.i(TAG, "getSurface:" + name);
        texture = new SurfaceTexture(name);
        texture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
            @Override
            public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                Log.e(TAG, "onFrameAvailable....");
            }
        });
        surface = new Surface(texture);
        HwDecodeWrapper.setOutputSurface(surface);
        return surface;
    }

    public static void updateTexImage(){
        texture.updateTexImage();
    }

    public static float[] getTransformMatrix(){
        texture.getTransformMatrix(matrix);
        return matrix;
    }

    public static void release(){
        texture.release();
        surface.release();
    }
}
