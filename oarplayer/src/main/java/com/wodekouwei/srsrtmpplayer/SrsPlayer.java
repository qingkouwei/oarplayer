package com.wodekouwei.srsrtmpplayer;

import android.os.Build;
import android.os.PowerManager;
import android.view.Surface;

import java.io.IOException;

/**
 * Created by shenjunwei on 2017/12/15.
 */

public class SrsPlayer {
    private final static String TAG = SrsPlayer.class.getName();

    private PowerManager.WakeLock mWakeLock = null;
    private boolean mScreenOnWhilePlaying;
    private boolean mStayAwake;

    private String mDataSource;

    private static volatile boolean mIsNativeInitialized = false;
    private static void initNativeOnce() {
        synchronized (SrsPlayer.class) {
            if (!mIsNativeInitialized) {
                native_init(Build.VERSION.SDK_INT, 44100);
                mIsNativeInitialized = true;
            }
        }
    }

    public SrsPlayer() {
        initPlayer();
    }
    private void initPlayer() {
        System.loadLibrary("oarp-lib");
        initNativeOnce();
        /*
        Looper looper;
        if ((looper = Looper.myLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else {
            mEventHandler = null;
        }*/

        /*
         * Native setup requires a weak reference to our object. It's easier to
         * create it here than in C++.
         */
//        native_setup(new WeakReference<SrsPlayer>(this));
    }
    private static native void native_init(int run_android_version, int best_samplerate);

    private native void _setDataSource(String path, String[] keys, String[] values)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException;

    public native void _prepareAsync() throws IllegalStateException;
    private native void _start() throws IllegalStateException;

    public void setDataSource(String path)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        mDataSource = path;
        _setDataSource(path, null, null);
    }

    public void prepareAsync() throws IllegalStateException {
        _prepareAsync();
    }
    public void start() throws IllegalStateException {
        stayAwake(true);
        _start();
    }

    public void stop() throws IllegalStateException {
        stayAwake(false);
        _stop();
    }
    public void setSurface(Surface surface){
        _setVideoSurface(surface);
    }

    private native void _stop() throws IllegalStateException;

    /*
         * Update the IjkMediaPlayer SurfaceTexture. Call after setting a new
         * display surface.
         */
    private native void _setVideoSurface(Surface surface);

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
        /*if (mSurfaceHolder != null) {
            mSurfaceHolder.setKeepScreenOn(mScreenOnWhilePlaying && mStayAwake);
        }*/
    }

    @SuppressWarnings("unused")
    void onPlayStatusChanged(int status) {

    }

    /**
     * native 调用的错误码回调
     *
     * @param error error code from native
     */
    void onPlayError(int error) {

    }
}
