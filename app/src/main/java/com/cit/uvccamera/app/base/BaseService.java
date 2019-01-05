package com.cit.uvccamera.app.base;

import android.app.Service;
import android.content.Context;
import android.util.Log;

public abstract class BaseService extends Service
{
    private static final boolean DEBUG = true;
    private static final String TAG = "BaseService";

    protected Context mContext = this;

    @Override
    public void onCreate()
    {
        super.onCreate();
        if (DEBUG) Log.d(TAG, "onCreate: [" + toString() + "]");
    }

    @Override
    public void onDestroy()
    {
        if (DEBUG) Log.d(TAG, "onDestroy: [" + toString() + "]");
        super.onDestroy();
    }
}
