package com.cit.uvccamera.aidl;

import android.hardware.usb.UsbDevice;
import android.view.Surface;

import java.util.List;

public interface ICore
{
    /**
     * @return 列出所有设备
     */
    List<UsbDevice> listUsbDevice();

    /**
     * 释放
     */
    void release();

    /**
     * 0: 绑定设备,默认选择一个
     * !0: 绑定指定设备
     *
     * @param id {@link android.hardware.usb.UsbDevice#getDeviceName#hashCode}
     */
    void select(int id);

    /**
     * 放弃选择
     */
    void unselect();

    /**
     * @param pixelformat (0:MJPEG; 1:H264)
     * @param fps         帧率
     * @param width       宽
     * @param height      高
     */
    void format(int pixelformat, int fps, int width, int height);

    /**
     * 添加预览
     *
     * @param surface surface
     */
    void addSurface(Surface surface);

    /**
     * 移除预览
     *
     * @param surface surface
     */
    void removeSurface(Surface surface);

    /**
     * 原始流数据
     *
     * @param callback null for cancel
     */
    void setCaptureCallback(ISharedCallback callback);

    /**
     * 解码流数据
     *
     * @param callback null for cancel
     */
    void setPreviewCallback(ISharedCallback callback);
}
