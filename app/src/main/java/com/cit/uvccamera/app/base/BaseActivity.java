package com.cit.uvccamera.app.base;

import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import butterknife.ButterKnife;
import butterknife.Unbinder;

public abstract class BaseActivity extends AppCompatActivity implements IBaseView
{
    private static final boolean DEBUG = true;
    private static final String TAG = "BaseActivity";

    protected Context mContext = this;
    protected Unbinder mUnbinder;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        if (DEBUG) Log.d(TAG, "onCreate: [" + toString() + "]");
        Bundle bundle = getIntent().getExtras();
        initData(bundle);
        setContentView(bindLayout());
        mUnbinder = ButterKnife.bind(this);
    }

    @Override
    protected void onDestroy()
    {
        if (DEBUG) Log.d(TAG, "onDestroy: [" + toString() + "]");
        if (mUnbinder != null)
        {
            mUnbinder.unbind();
            mUnbinder = null;
        }
        super.onDestroy();
    }
}
