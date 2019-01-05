// ISharedServiceCallback.aidl
package com.cit.uvccamera.aidl;

// Declare any non-default types here with import statements

interface ISharedServiceCallback
{
    oneway void onAdded(in ParcelFileDescriptor pfd, int length);

    oneway void onRemoved();

    oneway void onUpdated(int available);
}
