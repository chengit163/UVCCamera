package com.cit.uvccamera.app.activity;

import android.os.Bundle;
import android.support.annotation.Nullable;

import com.cit.uvccamera.app.R;
import com.cit.uvccamera.app.base.BaseActivity;
import com.cit.uvccamera.app.fragment.UVCFragment;

public class DoubleActivity extends BaseActivity
{
    @Override
    public void initData(@Nullable Bundle bundle)
    {

    }

    @Override
    public int bindLayout()
    {
        return R.layout.double_activity;
    }


    private UVCFragment mFirstFrament, mSecondFragment;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        mFirstFrament = new UVCFragment();
        getFragmentManager().beginTransaction().replace(R.id.first_rl, mFirstFrament).commit();
        mSecondFragment = new UVCFragment();
        getFragmentManager().beginTransaction().replace(R.id.second_rl, mSecondFragment).commit();
    }

    @Override
    protected void onDestroy()
    {
        getFragmentManager().beginTransaction().remove(mSecondFragment);
        mSecondFragment = null;
        getFragmentManager().beginTransaction().remove(mFirstFrament);
        mFirstFrament = null;
        super.onDestroy();
    }
}
