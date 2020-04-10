package com.sty.ne.livepushclient;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.sty.ne.livepushclient.util.PermissionUtils;

public class MainActivity extends AppCompatActivity {
//    private CameraHelper cameraHelper;
    private SurfaceView surfaceView;
    private Button btnSwitchCamera;
    private Button btnStartLive;
    private Button btnStopLive;
    private String[] needPermissions = {Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO};
    private LivePusher mPusher;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initView();
        setListeners();
        requestPermissions();
    }

    private void initView() {
        surfaceView = findViewById(R.id.surface_view);
        btnSwitchCamera = findViewById(R.id.btn_switch_camera);
        btnStartLive = findViewById(R.id.btn_start_live);
        btnStopLive = findViewById(R.id.btn_stop_live);

//        cameraHelper = new CameraHelper(this);
//        cameraHelper.setPreviewDisplay(surfaceView.getHolder());

        mPusher = new LivePusher(this);
        mPusher.setPreviewDisplay(surfaceView.getHolder());
    }

    private void setListeners() {
        btnSwitchCamera.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mPusher.switchCamera();
            }
        });
        btnStartLive.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onBtnStartLiveClicked();
            }
        });
        btnStopLive.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mPusher.stopLive();
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mPusher.release();
    }

    private void requestPermissions() {
        if(!PermissionUtils.checkPermissions(this, needPermissions)) {
            PermissionUtils.requestPermissions(this, needPermissions);
        }
    }

    private void onBtnStartLiveClicked() {
        if(PermissionUtils.checkPermissions(this, needPermissions)) {
            startLive();
        }else {
            PermissionUtils.requestPermissions(this, needPermissions);
        }
    }

    private void startLive() {
        mPusher.startLive("rtmp://47.115.6.127/myapp/");
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        if(requestCode == PermissionUtils.REQUEST_PERMISSIONS_CODE){
            if(!PermissionUtils.verifyPermissions(grantResults)) {
                PermissionUtils.showMissingPermissionDialog(this);
            }else {
                startLive();
            }
        }
    }
}
