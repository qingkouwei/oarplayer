package com.wodekouwei.srsrtmpplayer.proxy;

import android.media.MediaCodec;
import android.media.MediaFormat;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Created by shenjunwei on 2017/12/28.
 */

public class HwAudioDecodeWrapper {
    private final static String TAG = "HwAudioDecodeWrapper";
    private static MediaCodec codec = null;
    private static MediaFormat format = null;
    public static void init(String codecName,
                            int sample_rate,
                            int channelCount,
                            ByteBuffer csd0){
        Log.i(TAG, "init audio decodec: codecName = " + codecName);
        try {
            codec = MediaCodec.createDecoderByType(codecName);
            format = new MediaFormat();
            format.setString(MediaFormat.KEY_MIME, codecName);
            format.setInteger(MediaFormat.KEY_CHANNEL_COUNT, channelCount);
            format.setInteger(MediaFormat.KEY_SAMPLE_RATE, sample_rate);
//            format.setInteger(MediaFormat.KEY_IS_ADTS,1);
            format.setByteBuffer("csd-0", csd0);
            codec.configure(format, null, null, 0);
            codec.start();
            Log.i(TAG, "init audio decodec success...");
        } catch (IOException e) {
            e.printStackTrace();
            Log.e(TAG, "init audio decodec exception:" + e.getMessage());
        }
    }

    public static void stop(){
        codec.stop();
    }

    public static void flush(){
        codec.flush();
    }

    public static int dequeueInputBuffer(long timeout){
        return codec.dequeueInputBuffer(timeout);
    }

    public static ByteBuffer getInputBuffer(int id){
//        return codec.getInputBuffer(id);
        return codec.getInputBuffers()[id];
    }
    public static void queueInputBuffer(int id, int size, long pts, int flags){
        codec.queueInputBuffer(id, 0, size, pts, flags);
    }

    private static ByteBuffer bf = ByteBuffer.allocateDirect(20);
    private static ByteBuffer bf2 = ByteBuffer.allocateDirect(12);
    static{
        bf.order(ByteOrder.BIG_ENDIAN);
        bf2.order(ByteOrder.BIG_ENDIAN);
    }
    private static MediaCodec.BufferInfo output_buffer_info = new MediaCodec.BufferInfo();
    public static ByteBuffer dequeueOutputBufferIndex(long timeout){
        try {
            int id = codec.dequeueOutputBuffer(output_buffer_info, timeout);
//            Log.e(TAG, "dequeueOutputBuffer = " + id);
            bf.position(0);
            bf.putInt(id);
            if (id >= 0) {
                bf.putInt(output_buffer_info.offset);
                bf.putLong(output_buffer_info.presentationTimeUs);
                bf.putInt(output_buffer_info.size);
            }
        }catch (Exception e){
//            Log.e(TAG, "exception:" + e.getMessage());
        }
        return bf;
    }

    public static void releaseOutPutBuffer(int id){
//        Log.e(TAG, "outputBuffer id: " + id);
        try{
            codec.releaseOutputBuffer(id, false);
        }catch (Exception e){
            System.out.println("catch exception when releaseOutPutBuffer  id==>" + id);
            e.printStackTrace();
        }

    }

    public static ByteBuffer formatChange(){
        MediaFormat newFormat = codec.getOutputFormat();
//        Log.e(TAG, "format change:" + newFormat);
        int sample_rate = newFormat.getInteger(MediaFormat.KEY_SAMPLE_RATE);
        int channel_count = newFormat.getInteger(MediaFormat.KEY_CHANNEL_COUNT);
        Log.e(TAG, "newFormat:" + newFormat);
        //int pcm_encoding = newFormat.getInteger(MediaFormat.KEY_PCM_ENCODING);
        bf2.position(0);
        bf2.putInt(sample_rate);
        bf2.putInt(channel_count);
        bf2.putInt(1);
        return bf2;
    }

    public static ByteBuffer getOutputBuffer(int id){
//        ByteBuffer ret = codec.getOutputBuffer(id);
        return codec.getOutputBuffers()[id];
    }

    public static void release(){
        codec = null;
        format = null;
    }
}
