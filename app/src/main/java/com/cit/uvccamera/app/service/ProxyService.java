package com.cit.uvccamera.app.service;

import android.content.Intent;
import android.hardware.usb.UsbDevice;
import android.os.Binder;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.Surface;

import com.cit.uvccamera.aidl.CoreClient;
import com.cit.uvccamera.aidl.ICore;
import com.cit.uvccamera.aidl.ICoreCallback;
import com.cit.uvccamera.aidl.ISharedCallback;
import com.cit.uvccamera.app.BuildConfig;
import com.cit.uvccamera.app.base.BaseService;
import com.cit.uvccamera.app.event.Event;
import com.cit.uvccamera.app.event.EventFactory;
import com.cit.uvccamera.app.util.Wrap;

import org.greenrobot.eventbus.EventBus;

import java.util.List;

public class ProxyService extends BaseService implements ICore
{
    private static final boolean DEBUG = true;
    private static final String TAG = "ProxyService";

    public class ProxyServiceBinder extends Binder
    {
        public ProxyService getService()
        {
            return ProxyService.this;
        }
    }

    private ProxyServiceBinder myBinder = new ProxyServiceBinder();

    @Nullable
    @Override
    public IBinder onBind(Intent intent)
    {
        if (DEBUG) Log.d(TAG, "onBind: " + intent);
        return myBinder;
    }

    @Override
    public void onRebind(Intent intent)
    {
        super.onRebind(intent);
        if (DEBUG) Log.d(TAG, "onRebind: " + intent);
    }

    @Override
    public boolean onUnbind(Intent intent)
    {
        if (DEBUG) Log.d(TAG, "onUnbind: " + intent);
        return super.onUnbind(intent);
    }


    private ICore mCore;

    private ICoreCallback mCoreCallback = new ICoreCallback()
    {
        @Override
        public void onServiceConnected()
        {
            EventBus.getDefault().post(EventFactory.getEvent(Event.MSG_ON_SERVICE_CONNECTED));
        }

        @Override
        public void onServiceDisconnected()
        {
            EventBus.getDefault().post(EventFactory.getEvent(Event.MSG_ON_SERVICE_DISCONNECTED));
        }

        @Override
        public void onSelected()
        {
            EventBus.getDefault().post(EventFactory.getEvent(Event.MSG_ON_SELECTED));
        }

        @Override
        public void onUnselected()
        {
            EventBus.getDefault().post(EventFactory.getEvent(Event.MSG_ON_UNSELECTED));
        }

        @Override
        public void onConnected()
        {
            EventBus.getDefault().post(EventFactory.getEvent(Event.MSG_ON_CONNECTED));
        }

        @Override
        public void onDisConnected()
        {
            EventBus.getDefault().post(EventFactory.getEvent(Event.MSG_ON_DISCONNECTED));
        }

        @Override
        public void onFormat(int pixelformat, int fps, int width, int height)
        {
            Event event = EventFactory.getEvent(Event.MSG_ON_FORMAT);
            event.obj = new int[]{pixelformat, fps, width, height};
            EventBus.getDefault().post(event);
        }
    };

    @Override
    public void onCreate()
    {
        super.onCreate();
        // DEBUG版本打印日志
        if (BuildConfig.DEBUG)
            mCore = Wrap.wrap(new CoreClient(getApplicationContext(), Wrap.wrap(mCoreCallback)));
        else
            mCore = new CoreClient(getApplicationContext(), mCoreCallback);
    }

    @Override
    public void onDestroy()
    {
        if (mCore != null)
        {
            mCore.release();
            mCore = null;
        }
        super.onDestroy();
    }

    @Override
    public List<UsbDevice> listUsbDevice()
    {
        if (mCore != null)
            return mCore.listUsbDevice();
        return null;
    }

    @Override
    public void release()
    {
        Log.w(TAG, "ignore release");
    }

    @Override
    public void select(int id)
    {
        if (mCore != null)
            mCore.select(id);
    }

    @Override
    public void unselect()
    {
        if (mCore != null)
            mCore.unselect();
    }

    @Override
    public void format(int pixelformat, int fps, int width, int height)
    {
        if (mCore != null)
            mCore.format(pixelformat, fps, width, height);
    }

    @Override
    public void addSurface(Surface surface)
    {
        if (mCore != null)
            mCore.addSurface(surface);
    }

    @Override
    public void removeSurface(Surface surface)
    {
        if (mCore != null)
            mCore.removeSurface(surface);
    }

    @Override
    public void setCaptureCallback(ISharedCallback callback)
    {
        if (mCore != null)
            mCore.setCaptureCallback(callback);
    }

    @Override
    public void setPreviewCallback(ISharedCallback callback)
    {
        if (mCore != null)
            mCore.setPreviewCallback(callback);
    }
}
