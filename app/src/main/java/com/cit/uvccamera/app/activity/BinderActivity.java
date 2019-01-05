package com.cit.uvccamera.app.activity;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;

import com.cit.uvccamera.app.base.BaseActivity;
import com.cit.uvccamera.app.service.ProxyService;

import java.lang.ref.WeakReference;

public abstract class BinderActivity extends BaseActivity
{
    private static final boolean DEBUG = false;
    private static final String TAG = BinderActivity.class.getSimpleName();

    private final Object mServiceSync = new Object();
    private ProxyService mService;
    private WeakReference<Context> mWeakContext;


    private ServiceConnection mServiceConnection = new ServiceConnection()
    {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service)
        {
            if (DEBUG) Log.v(TAG, "onServiceConnected:name=" + name);
            synchronized (mServiceSync)
            {
                ProxyService.ProxyServiceBinder binder = (ProxyService.ProxyServiceBinder) service;
                mService = binder.getService();
                mServiceSync.notifyAll();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name)
        {
            if (DEBUG) Log.v(TAG, "onServiceDisconnected:name=" + name);
            synchronized (mServiceSync)
            {
                mService = null;
                mServiceSync.notifyAll();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        mWeakContext = new WeakReference<Context>(this);
        doBindService();
    }

    @Override
    protected void onDestroy()
    {
        doUnBindService();
        super.onDestroy();
    }

    private void doBindService()
    {
        if (DEBUG) Log.v(TAG, "doBindService:");
        synchronized (mServiceSync)
        {
            if (mService == null)
            {
                final Context context = mWeakContext.get();
                if (context != null)
                {
                    final Intent service = new Intent(this, ProxyService.class);
                    context.bindService(service, mServiceConnection, Context.BIND_AUTO_CREATE);
                }
            }
        }
    }

    private void doUnBindService()
    {
        if (DEBUG) Log.v(TAG, "doUnBindService:");
        synchronized (mServiceSync)
        {
            if (mService != null)
            {
                final Context context = mWeakContext.get();
                if (context != null)
                {
                    try
                    {
                        context.unbindService(mServiceConnection);
                    } catch (final Exception e)
                    {
                        // ignore
                    }
                }
                mService = null;
            }
        }
    }

    protected ProxyService getProxyService()
    {
        if (mService == null)
        {
            synchronized (mServiceSync)
            {
                if (mService == null)
                {
                    try
                    {
                        mServiceSync.wait();
                    } catch (final InterruptedException e)
                    {
                        e.printStackTrace();
                    }
                }
            }
        }
        return mService;
    }
}
