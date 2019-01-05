package com.cit.uvccamera.app.activity;

import android.app.AlertDialog;
import android.content.Intent;
import android.graphics.SurfaceTexture;
import android.hardware.usb.UsbDevice;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.cit.uvccamera.app.R;
import com.cit.uvccamera.app.event.Event;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class MainActivity extends BinderActivity
{
    private static final String TAG = "MainActivity";

    @BindView(R.id.main_rl)
    RelativeLayout mainRl;
    @BindView(R.id.sub_rl)
    RelativeLayout subRl;
    @BindView(R.id.format_tv)
    TextView formatTv;

    private SurfaceView mSurfaceView;
    private TextureView mTextureView;
    private Surface mSurface;

    @Override
    public void initData(@Nullable Bundle bundle)
    {
    }

    @Override
    public int bindLayout()
    {
        return R.layout.main_activity;
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        EventBus.getDefault().register(this);
    }

    @Override
    protected void onDestroy()
    {
        removePreviewCallback();
        removeCaptureCallback();
        EventBus.getDefault().unregister(this);
        super.onDestroy();
    }

    @OnClick({R.id.open_btn, R.id.close_btn,
            R.id.format_1280_720_btn, R.id.format_640_480_btn,
            R.id.add_surface_btn, R.id.remove_surface_btn,
            R.id.add_sub_btn, R.id.remove_sub_btn,
            R.id.add_capture_callback_btn, R.id.remove_capture_callback_btn,
            R.id.add_preview_callback_btn, R.id.remove_preview_callback_btn,
            R.id.double_btn})
    public void onClick(View v)
    {
        switch (v.getId())
        {
            case R.id.open_btn:
                List<UsbDevice> usbDevices = getProxyService().listUsbDevice();
                if (usbDevices != null && !usbDevices.isEmpty())
                {
                    String[] names = new String[usbDevices.size()];
                    for (int i = 0; i < names.length; i++)
                    {
                        names[i] = usbDevices.get(i).getDeviceName();
                    }
                    new AlertDialog.Builder(this).setItems(names, (dialog, which) -> getProxyService().select(names[which].hashCode())).create().show();
                } else
                {
                    Toast.makeText(this, "on support device", Toast.LENGTH_SHORT).show();
                }
                break;
            case R.id.close_btn:
                getProxyService().unselect();
                break;
            case R.id.format_1280_720_btn:
                getProxyService().format(-1, -1, 1280, 720);
                break;
            case R.id.format_640_480_btn:
                getProxyService().format(-1, -1, 640, 480);
                break;
            case R.id.add_surface_btn:
                addSurface();
                break;
            case R.id.remove_surface_btn:
                removeSurface();
                break;
            case R.id.add_sub_btn:
                addSub();
                break;
            case R.id.remove_sub_btn:
                removeSub();
                break;
            case R.id.add_capture_callback_btn:
                addCaptureCallback();
                break;
            case R.id.remove_capture_callback_btn:
                removeCaptureCallback();
                break;
            case R.id.add_preview_callback_btn:
                addPreviewCallback();
                break;
            case R.id.remove_preview_callback_btn:
                removePreviewCallback();
                break;
            case R.id.double_btn:
                startActivity(new Intent(this, DoubleActivity.class));
                break;
        }
    }


    private void addCaptureCallback()
    {
        getProxyService().setCaptureCallback((data, available) ->
                Log.d(TAG, data.length + " >= " + available));
    }

    private void removeCaptureCallback()
    {
        getProxyService().setCaptureCallback(null);
    }

    private void addPreviewCallback()
    {
        getProxyService().setPreviewCallback((data, available) ->
                Log.d(TAG, data.length + " >= " + available));
    }

    private void removePreviewCallback()
    {
        getProxyService().setPreviewCallback(null);
    }


    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEvent(Event event)
    {
        switch (event.what)
        {
            case Event.MSG_ON_SERVICE_CONNECTED:
                break;
            case Event.MSG_ON_SERVICE_DISCONNECTED:
                removeSub();
                removeSurface();
                formatTv.setText("");
                break;
            case Event.MSG_ON_SELECTED:
                break;
            case Event.MSG_ON_UNSELECTED:
                removeSub();
                removeSurface();
                formatTv.setText("");
                break;
            case Event.MSG_ON_CONNECTED:
                addSurface();
                break;
            case Event.MSG_ON_DISCONNECTED:
                removeSub();
                removeSurface();
                formatTv.setText("");
                break;
            case Event.MSG_ON_FORMAT:
                int[] params = (int[]) event.obj;
                String text = String.format("(fps%d, %d x %d)", params[1], params[2], params[3]);
                formatTv.setText(text);
                break;
        }
    }

    private void addSurface()
    {
        if (mSurfaceView == null)
        {
            mSurfaceView = new SurfaceView(this);
            mSurfaceView.getHolder().addCallback(new SurfaceHolder.Callback()
            {
                @Override
                public void surfaceDestroyed(SurfaceHolder holder)
                {
                    getProxyService().removeSurface(holder.getSurface());
                }

                @Override
                public void surfaceCreated(SurfaceHolder holder)
                {
                    getProxyService().addSurface(holder.getSurface());
                }

                @Override
                public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
                {
                }
            });
            RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT,
                    RelativeLayout.LayoutParams.MATCH_PARENT);
            mSurfaceView.setLayoutParams(params);
            mainRl.addView(mSurfaceView);
        }
    }

    private void removeSurface()
    {
        if (mSurfaceView != null)
        {
            mainRl.removeView(mSurfaceView);
            mSurfaceView = null;
        }
    }

    private void addSub()
    {
        if (mTextureView == null)
        {
            mTextureView = new TextureView(this);
            mTextureView.setSurfaceTextureListener(new TextureView.SurfaceTextureListener()
            {
                @Override
                public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height)
                {
                    mSurface = new Surface(surface);
                    getProxyService().addSurface(mSurface);
                }

                @Override
                public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height)
                {
                }

                @Override
                public boolean onSurfaceTextureDestroyed(SurfaceTexture surface)
                {
                    if (mSurface != null)
                    {
                        getProxyService().removeSurface(mSurface);
                        mSurface = null;
                    }
                    return false;
                }

                @Override
                public void onSurfaceTextureUpdated(SurfaceTexture surface)
                {
                }
            });
            RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT,
                    RelativeLayout.LayoutParams.MATCH_PARENT);
            mTextureView.setLayoutParams(params);
            subRl.addView(mTextureView);
        }
    }

    private void removeSub()
    {
        if (mTextureView != null)
        {
            subRl.removeView(mTextureView);
            mTextureView = null;
        }
    }
}
