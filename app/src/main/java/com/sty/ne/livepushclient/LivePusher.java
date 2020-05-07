package com.sty.ne.livepushclient;

import android.app.Activity;
import android.view.SurfaceHolder;

public class LivePusher {
    private final VideoChannel mVideoChannel;
    private final AudioChannel mAudioChannel;

    public LivePusher(Activity activity) {
        initNative();
        mVideoChannel = new VideoChannel(activity, this);
        mAudioChannel = new AudioChannel(this);
    }

    public void switchCamera() {
        mVideoChannel.switchCamera();
    }

    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        mVideoChannel.setPreviewDisplay(surfaceHolder);
    }

    public void rePreview() {
        mVideoChannel.rePreview();
    }

    /**
     * 开始直播，先准备好编码器
     * @param path rtmp推流地址： rtmp://47.115.6.127/myapp/
     */
    public void startLive(String path) {
        startLiveNative(path);
        mVideoChannel.startLive();
        mAudioChannel.startLive();
    }

    /**
     * 停止直播
     */
    public void stopLive() {
        mVideoChannel.stopLive();
        mAudioChannel.stopLive();
        stopLiveNative();
    }

    public void release() {
        mVideoChannel.release();
        mAudioChannel.release();
        releaseNative();
    }

    //初始化，准备队列
    private native void initNative();
    //建立服务器连接 开子线程，循环从队列中读取rtmp包
    private native void startLiveNative(String path);
    //停止直播，断开服务器连接，跳出子线程循环
    private native void stopLiveNative();
    //初始化视频编码器（设置视频相关参数，width, height, bitrate, fps）
    public native void initVideoEncoderNative(int width, int height, int bitrate, int fps);
    //推送视频数据（进行H.264编码）
    public native void pushVideoNative(byte[] data);
    //释放
    public native void releaseNative();
    //初始化音频编码器
    public native void initAudioEncoderNative(int sampleRateInHz, int channelConfig);
    //推送音频数据（进行aac编码） 录音机
    public native void pushAudioNative(byte[] data);

    public native int getInputSamplesNative();

    static {
        System.loadLibrary("native-lib");
    }


}
