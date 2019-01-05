package com.cit.uvccamera.app.base;

import android.os.Bundle;
import android.support.annotation.Nullable;

public interface IBaseView
{
    /**
     * 初始化数据
     *
     * @param bundle 传递过来的 bundle
     */
    void initData(@Nullable final Bundle bundle);

    /**
     * 绑定布局
     *
     * @return 布局 Id
     */
    int bindLayout();
}
