package com.sty.ne.livepushclient;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioChannel {
    private final LivePusher mPusher;
    private int channels = 2;
    private AudioRecord audioRecord;
    private int inputSamples; //单位：字节
    private boolean isLive;
    private ExecutorService executorService;

    public AudioChannel(LivePusher pusher) {
        mPusher = pusher;
        int channelConfig;
        if(channels == 2) {
            channelConfig = AudioFormat.CHANNEL_IN_STEREO; //双声道
        }else {
            channelConfig = AudioFormat.CHANNEL_IN_MONO; //单声道
        }

        executorService = Executors.newSingleThreadExecutor();
        //先进行音频编码器的初始化
        pusher.initAudioEncoderNative(44100, channels); //第二个参数是声道数
        int minBufferSize = AudioRecord.getMinBufferSize(44100, channelConfig,
                AudioFormat.ENCODING_PCM_16BIT) * 2; //避免溢出，通常*2
        //获取编码器的输入样本数
        inputSamples = pusher.getInputSamplesNative() * 2; //16bit = 2字节

        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, 44100, channelConfig,
                AudioFormat.ENCODING_PCM_16BIT, Math.max(minBufferSize, inputSamples));
//        audioRecord.startRecording();
//        audioRecord.read();
//        audioRecord.stop();
    }

    public void startLive() {
        isLive = true;
        executorService.submit(new AudioTask());
    }

    public void stopLive() {
        isLive = false;
    }

    public void release() {
        if(audioRecord != null) {
            audioRecord.release();
            audioRecord = null;
        }
    }

    private class AudioTask implements Runnable{
        @Override
        public void run() {
            audioRecord.startRecording(); //开始录音
            byte[] data = new byte[inputSamples];
            while (isLive) {
                //每次读多少根据编码器实际支持的输入样本数据大小来定
                int length = audioRecord.read(data, 0, data.length);
                if(length > 0) {
                    mPusher.pushAudioNative(data);
                }
            }
            audioRecord.stop(); //停止录音
        }
    }
}
