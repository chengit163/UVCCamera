package com.cit.uvccamera.app.base;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import butterknife.ButterKnife;
import butterknife.Unbinder;

public abstract class BaseFragment extends Fragment implements IBaseView
{
    private static final boolean DEBUG = true;
    private static final String TAG = "BaseFragment";

    protected View mView;
    protected Activity mActivity;
    protected Unbinder mUnbinder;


    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, Bundle savedInstanceState)
    {
        if (DEBUG) Log.d(TAG, "onCreateView: [" + toString() + "]");
        mView = inflater.inflate(bindLayout(), null);
        mUnbinder = ButterKnife.bind(this, mView);
        return mView;
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState)
    {
        if (DEBUG) Log.d(TAG, "onViewCreated: [" + toString() + "]");
        super.onViewCreated(view, savedInstanceState);
        Bundle bundle = getArguments();
        initData(bundle);
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState)
    {
        if (DEBUG) Log.d(TAG, "onActivityCreated: [" + toString() + "]");
        super.onActivityCreated(savedInstanceState);
        mActivity = getActivity();
    }

    @Override
    public void onDestroyView()
    {
        if (DEBUG) Log.d(TAG, "onDestroyView: [" + toString() + "]");
        if (mUnbinder != null)
        {
            mUnbinder.unbind();
            mUnbinder = null;
        }
        super.onDestroyView();
    }
}
