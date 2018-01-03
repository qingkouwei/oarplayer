package com.wodekouwei.demo;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

import com.wodekouwei.srsrtmpplayer.OARPlayer;

import java.io.IOException;

public class MainActivity extends AppCompatActivity {


    Button btn;
    private SurfaceView surfaceView;
    OARPlayer player;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        btn = (Button)findViewById(R.id.btn);
        player = new OARPlayer();
        surfaceView = (SurfaceView)findViewById(R.id.surfaceview);
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder surfaceHolder) {

            }

            @Override
            public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

            }
        });
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
//                    player.setDataSource("rtmp://192.168.31.34/live/aaaaa");
                    player.setDataSource("rtmp://192.168.0.49/live/aaaaa");
                    player.setSurface(surfaceView.getHolder().getSurface());
                    player.start();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                /*new Thread(){
                    @Override
                    public void run() {
                    }
                }.start();*/

            }
        });
    }
}
