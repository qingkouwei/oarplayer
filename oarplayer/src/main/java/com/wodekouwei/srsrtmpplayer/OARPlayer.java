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
package com.wodekouwei.srsrtmpplayer;

import android.content.Context;
import android.os.Build;
import android.os.PowerManager;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.io.IOException;

/**
 * Created by qingkouwei on 2017/12/15.
 */

public class OARPlayer {
    private final static String TAG = OARPlayer.class.getName();
    private final static boolean isDebug = false;
    private PowerManager.WakeLock mWakeLock = null;
    private boolean mScreenOnWhilePlaying;
    private boolean mStayAwake;

    private String mDataSource;
    private SurfaceHolder mSurfaceHolder;

    private volatile boolean mIsNativeInitialized = false;
    private void initNativeOnce() {
        synchronized (OARPlayer.class) {
            if (!mIsNativeInitialized) {
                native_init(Build.VERSION.SDK_INT, 44100);
                mIsNativeInitialized = true;
            }
        }
    }

    public OARPlayer() {
        initPlayer();
    }
    private void initPlayer() {
        System.loadLibrary("oarp-lib");
        initNativeOnce();
    }
    private native void native_init(int run_android_version, int best_samplerate);

    /**
     * setting rtmp url
     * @param path
     * @throws IOException
     * @throws IllegalArgumentException
     * @throws SecurityException
     * @throws IllegalStateException
     */
    public void setDataSource(String path)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        mDataSource = path;
        _setDataSource(path, null, null);
    }
    private native void _setDataSource(String path, String[] keys, String[] values)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException;

    /**
     * unused
     * @throws IllegalStateException
     */
    @SuppressWarnings("unused")
    public void prepareAsync() throws IllegalStateException {
        _prepareAsync();
    }
    private native void _prepareAsync() throws IllegalStateException;

    /**
     * setting reading packet buffer time
     * @param time
     */
    public void setBufferTime(float time){
        _setBufferTime(time);
    }
    private native void _setBufferTime(float time);

    /**
     * start play
     * @throws IllegalStateException
     */
    public void start() throws IllegalStateException {
        stayAwake(true);
        _start();
    }
    private native void _start() throws IllegalStateException;

    /**
     * setting playbackground(only play audio)
     * @param playBackground
     */
    public void setPlayBackground(boolean playBackground) {
        _setPlayBackground(playBackground);
    }
    private native void _setPlayBackground(boolean playBackground);

    /**
     * called in onPause of Activity
     */
    public void onPause() {
        _onPause();
    }
    private native void _onPause();
    public void onResume() {
        _onResume();
    }
    private native void _onResume();

    /**
     * stop play
     * @throws IllegalStateException
     */
    public void stop() throws IllegalStateException {
        stayAwake(false);
        _stop();
    }
    private native void _stop() throws IllegalStateException;

    /**
     * release res
     */
    public void release(){
        stayAwake(false);
        _release();
    }
    private native void _release();

    /**
     * setting render surface
     * @param holder
     */
    public void setSurface(SurfaceHolder holder){
        mSurfaceHolder = holder;
        _setVideoSurface(holder.getSurface());
    }
    /*
     * Update the SurfaceTexture. Call after setting a new
     * display surface.
     */
    private native void _setVideoSurface(Surface surface);

    /**
     * get current play time
     * @return
     */
    public float getCurrentTime(){
        return _getCurrentTime();
    }
    private native float _getCurrentTime();

    public void setWakeMode(Context context, int mode) {
        boolean washeld = false;
        if (mWakeLock != null) {
            if (mWakeLock.isHeld()) {
                washeld = true;
                mWakeLock.release();
            }
            mWakeLock = null;
        }

        PowerManager pm = (PowerManager) context
                .getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(mode | PowerManager.ON_AFTER_RELEASE,
                OARPlayer.class.getName());
        mWakeLock.setReferenceCounted(false);
        if (washeld) {
            mWakeLock.acquire();
        }
    }
    public void setScreenOnWhilePlaying(boolean screenOn) {
        if (mScreenOnWhilePlaying != screenOn) {
            if (screenOn && mSurfaceHolder == null) {
                Log.w(TAG,
                        "setScreenOnWhilePlaying(true) is ineffective without a SurfaceHolder");
            }
            mScreenOnWhilePlaying = screenOn;
            updateSurfaceScreenOn();
        }
    }

    private void stayAwake(boolean awake) {
        if (mWakeLock != null) {
            if (awake && !mWakeLock.isHeld()) {
                mWakeLock.acquire();
            } else if (!awake && mWakeLock.isHeld()) {
                mWakeLock.release();
            }
        }
        mStayAwake = awake;
        updateSurfaceScreenOn();
    }

    private void updateSurfaceScreenOn() {
        if (mSurfaceHolder != null) {
            mSurfaceHolder.setKeepScreenOn(mScreenOnWhilePlaying && mStayAwake);
        }
    }

    /**
     * callback by jni
     * @param status
     */
    void onPlayStatusChanged(int status) {
        if(isDebug) Log.i(TAG, "onPlayStatusChanged:" + status);
    }

    /**
     * native 调用的错误码回调
     *
     * @param error error code from native
     */
    void onPlayError(int error) {
        if(isDebug) Log.i(TAG, "onPlayError:" + error);
    }
}
