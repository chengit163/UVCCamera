// ICoreService.aidl
package com.cit.uvccamera.aidl;

import android.hardware.usb.UsbDevice;
import android.view.Surface;
import com.cit.uvccamera.aidl.ICoreServiceCallback;
import com.cit.uvccamera.aidl.ISharedServiceCallback;

// Declare any non-default types here with import statements

interface ICoreService
{
    List<UsbDevice> listUsbDevice();

    void select(int id, ICoreServiceCallback callback);

    void unselect(int id, ICoreServiceCallback callback);

    void format(int id, int pixelformat, int fps, int width, int height);

    void addSurface(int id, int surface_id, in Surface surface);

    void removeSurface(int id, int surface_id);

    void addCaptureCallback(int id, ISharedServiceCallback callback);

    void removeCaptureCallback(int id, ISharedServiceCallback callback);

    void addPreviewCallback(int id, ISharedServiceCallback callback);

    void removePreviewCallback(int id, ISharedServiceCallback callback);
}
