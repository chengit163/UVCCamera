package com.cit.uvccamera.aidl;

public interface ISharedCallback
{
    void onUpdated(final byte[] data, final int available);
}
