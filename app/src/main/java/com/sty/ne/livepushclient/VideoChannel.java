package com.sty.ne.livepushclient;

import android.app.Activity;
import android.hardware.Camera;
import android.view.SurfaceHolder;

public class VideoChannel implements CameraHelper.OnChangedSizeListener, Camera.PreviewCallback{
    private static final int WIDTH = 800;
    private static final int HEIGHT = 480;
    private static final int CAMERA_ID = Camera.CameraInfo.CAMERA_FACING_FRONT;
    private static final int FPS = 25;
    private static final int BITRATE = 800000;
    private LivePusher mPusher;

    private CameraHelper cameraHelper;
    private boolean isLive;

    public VideoChannel(Activity activity, LivePusher mPusher) {
        this.mPusher = mPusher;
        this.cameraHelper = new CameraHelper(activity, CAMERA_ID, WIDTH, HEIGHT);
        cameraHelper.setOnChangedSizeListener(this);
        cameraHelper.setPreviewCallback(this);
    }

    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        cameraHelper.setPreviewDisplay(surfaceHolder);
    }

    public void switchCamera() {
        cameraHelper.switchCamera();
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        //推送视频数据
        if(isLive) {
            mPusher.pushVideoNative(data);
        }
    }

    @Override
    public void onChanged(int w, int h) {
        //初始化视频编码器
        mPusher.initVideoEncoderNative(w, h, BITRATE, FPS);
    }

    public void startLive() {
        isLive = true;
    }

    public void stopLive() {
        isLive = false;
    }

    public void release() {
        cameraHelper.stopPreview();
    }
}
