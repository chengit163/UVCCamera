package com.cit.uvccamera.aidl;

public interface ICoreCallback
{
    /**
     * 与Service建立连接
     */
    void onServiceConnected();

    /**
     * 与Service断开连接
     */
    void onServiceDisconnected();

    /**
     * 绑定操作设备
     */
    void onSelected();

    /**
     * 解绑操作设备
     */
    void onUnselected();

    /**
     * 设备已Open
     */
    void onConnected();

    /**
     * 设备已Close
     */
    void onDisConnected();

    /**
     * @param pixelformat
     * @param fps
     * @param width
     * @param height
     */
    void onFormat(int pixelformat, int fps, int width, int height);
}
