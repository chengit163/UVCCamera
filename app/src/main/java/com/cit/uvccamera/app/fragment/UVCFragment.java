package com.cit.uvccamera.app.fragment;

import android.app.AlertDialog;
import android.graphics.SurfaceTexture;
import android.hardware.usb.UsbDevice;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.cit.uvccamera.aidl.CoreClient;
import com.cit.uvccamera.aidl.ICore;
import com.cit.uvccamera.aidl.ICoreCallback;
import com.cit.uvccamera.app.R;
import com.cit.uvccamera.app.base.BaseFragment;

import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class UVCFragment extends BaseFragment
{

    private ICore mCore;
    private ICoreCallback mCoreCallback = new ICoreCallback()
    {
        @Override
        public void onServiceConnected()
        {
        }

        @Override
        public void onServiceDisconnected()
        {
            mActivity.runOnUiThread(() ->
            {
                removeSubSurface();
                removeMainSurface();
            });
        }

        @Override
        public void onSelected()
        {
        }

        @Override
        public void onUnselected()
        {
            mActivity.runOnUiThread(() ->
            {
                removeSubSurface();
                removeMainSurface();
            });
        }

        @Override
        public void onConnected()
        {
            mActivity.runOnUiThread(() -> addMainSurface());
        }

        @Override
        public void onDisConnected()
        {
            mActivity.runOnUiThread(() ->
            {
                removeSubSurface();
                removeMainSurface();
            });
        }

        @Override
        public void onFormat(int pixelformat, int fps, int width, int height)
        {
        }
    };

    @BindView(R.id.main_rl)
    RelativeLayout mainRl;
    @BindView(R.id.sub_rl)
    RelativeLayout subRl;

    private SurfaceView mMainSurfaceView;
    private SurfaceView mSubSurfaceView;


    @Override
    public void initData(@Nullable Bundle bundle)
    {
    }

    @Override
    public int bindLayout()
    {
        return R.layout.uvc_fragment;
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState)
    {
        super.onActivityCreated(savedInstanceState);
        if (mCore == null)
        {
            mCore = new CoreClient(mActivity, mCoreCallback);
        }
    }

    @Override
    public void onDestroyView()
    {
        removeSubSurface();
        removeMainSurface();
        if (mCore != null)
        {
            mCore.release();
            mCore = null;
        }
        super.onDestroyView();
    }

    @OnClick({R.id.open_btn, R.id.close_btn, R.id.add_sub_btn, R.id.remove_sub_btn})
    public void onClick(View v)
    {
        switch (v.getId())
        {
            case R.id.open_btn:
                List<UsbDevice> usbDevices = mCore.listUsbDevice();
                if (usbDevices != null && !usbDevices.isEmpty())
                {
                    String[] names = new String[usbDevices.size()];
                    for (int i = 0; i < names.length; i++)
                    {
                        names[i] = usbDevices.get(i).getDeviceName();
                    }
                    new AlertDialog.Builder(mActivity).setItems(names, (dialog, which) -> mCore.select(names[which].hashCode())).create().show();
                } else
                {
                    Toast.makeText(mActivity, "on support device", Toast.LENGTH_SHORT).show();
                }
                break;
            case R.id.close_btn:
                mCore.unselect();
                break;
            case R.id.add_sub_btn:
                addSubSurface();
                break;
            case R.id.remove_sub_btn:
                removeSubSurface();
                break;
        }
    }


    private void addMainSurface()
    {
        if (mMainSurfaceView == null)
        {
            mMainSurfaceView = new SurfaceView(mActivity);
            mMainSurfaceView.getHolder().addCallback(new SurfaceHolder.Callback()
            {
                @Override
                public void surfaceDestroyed(SurfaceHolder holder)
                {
                    if (mCore != null)
                        mCore.removeSurface(holder.getSurface());
                }

                @Override
                public void surfaceCreated(SurfaceHolder holder)
                {
                    if (mCore != null)
                        mCore.addSurface(holder.getSurface());
                }

                @Override
                public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
                {
                }
            });
            RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT,
                    RelativeLayout.LayoutParams.MATCH_PARENT);
            mMainSurfaceView.setLayoutParams(params);
            mainRl.addView(mMainSurfaceView);
        }
    }

    private void removeMainSurface()
    {
        if (mMainSurfaceView != null)
        {
            mainRl.removeView(mMainSurfaceView);
            mMainSurfaceView = null;
        }
    }

    private void addSubSurface()
    {
        if (mSubSurfaceView == null)
        {
            mSubSurfaceView = new SurfaceView(mActivity);
            mSubSurfaceView.getHolder().addCallback(new SurfaceHolder.Callback()
            {
                @Override
                public void surfaceDestroyed(SurfaceHolder holder)
                {
                    if (mCore != null)
                        mCore.removeSurface(holder.getSurface());
                }

                @Override
                public void surfaceCreated(SurfaceHolder holder)
                {
                    if (mCore != null)
                        mCore.addSurface(holder.getSurface());
                }

                @Override
                public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
                {
                }
            });
            RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT,
                    RelativeLayout.LayoutParams.MATCH_PARENT);
            mSubSurfaceView.setLayoutParams(params);
            subRl.addView(mSubSurfaceView);
        }
    }

    private void removeSubSurface()
    {
        if (mSubSurfaceView != null)
        {
            subRl.removeView(mSubSurfaceView);
            mSubSurfaceView = null;
        }
    }
}
