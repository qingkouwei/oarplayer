/*
 The MIT License (MIT)

Copyright (c) 2017-2020 oarplayer(qingkouwei)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
package com.wodekouwei.srsrtmpplayer.proxy;

import android.graphics.SurfaceTexture;
import android.util.Log;
import android.view.Surface;

public class SurfaceTextureWrapper {
    private final static String TAG = "SurfaceTextureWrapper";
    private final static boolean isDebug = false;
    private static SurfaceTexture texture;
    private static Surface surface;
    private static float[] matrix = new float[16];

    public static Surface getSurface(int name){
        if(isDebug) Log.i(TAG, "getSurface:" + name);
        texture = new SurfaceTexture(name);
        /*texture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
            @Override
            public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                if(isDebug) Log.e(TAG, "onFrameAvailable....");
            }
        });*/
        surface = new Surface(texture);
        HwVideoDecodeWrapper.setOutputSurface(surface);
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
