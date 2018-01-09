package com.wodekouwei.demo;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;

import com.wodekouwei.srsrtmpplayer.OARPlayer;

import java.io.IOException;

/**
 * Created by shenjunwei on 2018/1/5.
 */

public class PlayerAty extends Activity {

    private SurfaceView surfaceView;
    private TextView playTime;
    OARPlayer player;
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        setContentView(R.layout.activity_player);
        player = new OARPlayer();
        surfaceView = (SurfaceView)findViewById(R.id.surfaceview);
        playTime = (TextView)findViewById(R.id.playTime);
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder surfaceHolder) {
                try {
                    String rtmp_url = getIntent().getStringExtra("rtmp_url");
                    rtmp_url = "rtmp://192.168.0.49/live/aaaaa";
                    player.setDataSource(rtmp_url);
                    playTimeHandler.sendEmptyMessageDelayed(1, 1000);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                player.setSurface(surfaceView.getHolder());
                player.start();
            }

            @Override
            public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

            }
        });
    }
    private Handler playTimeHandler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            playTime.setText((int) player.getCurrentTime() + "");
            playTimeHandler.sendEmptyMessageDelayed(1, 1000);
        }
    };

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onStop() {
        player.stop();
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        playTimeHandler.removeMessages(1);
        player.release();
        super.onDestroy();
    }
}
