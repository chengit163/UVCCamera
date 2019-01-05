package com.cit.uvccamera;

import android.os.MemoryFile;
import android.os.ParcelFileDescriptor;
import android.view.Surface;

import java.io.FileDescriptor;
import java.lang.reflect.Method;

public class UVCCamera
{
    private static boolean isLoaded;

    static
    {
        if (!isLoaded)
        {
            System.loadLibrary("jpeg-turbo-1.5.3");
            System.loadLibrary("usb");
            System.loadLibrary("uvc");
            System.loadLibrary("UVCCamera");
            isLoaded = true;
        }
    }

    private static final int PIXEL_FORMAT_MJPEG = 0;
    private static final int PIXEL_FORMAT_H264 = 1;

    private static final int DEFAULT_FRAME_FPS = 30;
    private static final int DEFAULT_FRAME_WIDTH = 1280;
    private static final int DEFAULT_FRAME_HEIGHT = 720;

    /**
     * MJPEG
     */
    public static final int CAPTURE_SIZE = 256 * 1024 + 16;
    /**
     * YUV
     */
    public static final int PREVIEW_SIZE = DEFAULT_FRAME_WIDTH * DEFAULT_FRAME_HEIGHT * 3 / 2 + 16;

    protected long mNativePtr;
    protected int mPixelformat = PIXEL_FORMAT_MJPEG;

    public UVCCamera()
    {
        mNativePtr = nativeCreate();
    }

    public boolean isConnected()
    {
        boolean result = false;
        if (0 != mNativePtr)
        {
            result = nativeIsConnected(mNativePtr);
        }
        return result;
    }

    public boolean connect(String name, int fd)
    {
        boolean result = false;
        if (0 != mNativePtr)
        {
            result = nativeConnect(mNativePtr, name, fd);
            if (result)
            {
                nativeFormat(mNativePtr, mPixelformat, DEFAULT_FRAME_FPS, DEFAULT_FRAME_WIDTH, DEFAULT_FRAME_HEIGHT);
            }
        }
        return result;
    }

    public void release()
    {
        if (0 != mNativePtr)
        {
//            nativePreviewCloseShared(mNativePtr);
//            nativeStopPreview(mNativePtr);
//            nativeClearPreviewDisplay(mNativePtr);
//            nativeCaptureCloseShared(mNativePtr);
//            nativeStopCapture(mNativePtr);
            nativeRelease(mNativePtr);
            nativeDestroy(mNativePtr);
        }
    }

    public boolean format(int pixelformat, int fps, int width, int height)
    {
        boolean result = false;
        if (0 != mNativePtr)
        {
            mPixelformat = (-1 == pixelformat ? PIXEL_FORMAT_MJPEG : pixelformat);
            fps = (-1 == fps ? DEFAULT_FRAME_FPS : fps);
            width = (-1 == width ? DEFAULT_FRAME_WIDTH : width);
            height = (-1 == height ? DEFAULT_FRAME_HEIGHT : height);
            result = nativeFormat(mNativePtr, mPixelformat, fps, width, height);
        }
        return result;
    }

    public int getPixelformat()
    {
        int pixelformat = PIXEL_FORMAT_MJPEG;
        if (0 != mNativePtr)
        {
            pixelformat = nativeGetPixelformat(mNativePtr);
        }
        return pixelformat;
    }

    public int getFps()
    {
        int fps = DEFAULT_FRAME_FPS;
        if (0 != mNativePtr)
        {
            fps = nativeGetFps(mNativePtr);
        }
        return fps;
    }

    public int getWidth()
    {
        int width = DEFAULT_FRAME_WIDTH;
        if (0 != mNativePtr)
        {
            width = nativeGetWidth(mNativePtr);
        }
        return width;
    }

    public int getHeight()
    {
        int height = DEFAULT_FRAME_WIDTH;
        if (0 != mNativePtr)
        {
            height = nativeGetHeight(mNativePtr);
        }
        return height;
    }


    public void addPreviewDisplay(int id, Surface surface)
    {
        if (0 != mNativePtr)
        {
            nativeAddPreviewDisplay(mNativePtr, id, surface);
            if (nativeCountPreviewDisplay(mNativePtr) > 0)
            {
                nativeStartCapture(mNativePtr);
                nativeStartPreview(mNativePtr);
            }
        }
    }

    public void removePreviewDisplay(int id)
    {
        if (0 != mNativePtr)
        {
            nativeRemovePreviewDisplay(mNativePtr, id);
            if (nativeCountPreviewDisplay(mNativePtr) == 0)
            {
                nativeStopPreview(mNativePtr);
                if (!nativeIsPreview(mNativePtr))
                    nativeStopCapture(mNativePtr);
            }
        }
    }

//    public int countPreviewDisplay()
//    {
//        int result = 0;
//        if (0 != mNativePtr)
//        {
//            result = nativeCountPreviewDisplay(mNativePtr);
//        }
//        return result;
//    }


    public void setFrameCallback(IFrameCallback frameCallback)
    {
        if (0 != mNativePtr)
        {
            nativeSetFrameCallback(mNativePtr, frameCallback);
            if (frameCallback != null)
            {
                nativeStartCapture(mNativePtr);
            } else
            {
                if (!nativeIsPreview(mNativePtr))
                    nativeStopCapture(mNativePtr);
            }
        }
    }

    public MemoryFile captureOpenShared(int length, ISharedCallback sharedCallback)
    {
        MemoryFile memoryFile = null;
        if (0 != mNativePtr)
        {
            memoryFile = nativeCaptureOpenShared(mNativePtr, length, sharedCallback);
            if (memoryFile != null)
            {
                nativeStartCapture(mNativePtr);
            }
        }
        return memoryFile;
    }

    public void captureCloseShared()
    {
        if (0 != mNativePtr)
        {
            nativeCaptureCloseShared(mNativePtr);
            if (nativeCountPreviewDisplay(mNativePtr) == 0)
            {
                if (!nativeIsPreview(mNativePtr))
                    nativeStopCapture(mNativePtr);
            }
        }
    }

    public ParcelFileDescriptor captureDupShared(int length, ISharedCallback sharedCallback)
    {
        MemoryFile memoryFile = captureOpenShared(length, sharedCallback);
        return dup(memoryFile);
    }


    public MemoryFile previewOpenShared(int length, ISharedCallback sharedCallback)
    {
        MemoryFile memoryFile = null;
        if (0 != mNativePtr)
        {
            memoryFile = nativePreviewOpenShared(mNativePtr, length, sharedCallback);
            if (memoryFile != null)
            {
                nativeStartCapture(mNativePtr);
                nativeStartPreview(mNativePtr);
            }
        }
        return memoryFile;
    }

    public void previewCloseShared()
    {
        if (0 != mNativePtr)
        {
            nativePreviewCloseShared(mNativePtr);
            if (nativeCountPreviewDisplay(mNativePtr) == 0)
            {
                nativeStopPreview(mNativePtr);
                if (!nativeIsPreview(mNativePtr))
                    nativeStopCapture(mNativePtr);
            }
        }
    }

    public ParcelFileDescriptor previewDupShared(int length, ISharedCallback sharedCallback)
    {
        MemoryFile memoryFile = previewOpenShared(length, sharedCallback);
        return dup(memoryFile);
    }


    /**
     * aidl传输的文件描述符
     *
     * @param memoryFile
     * @return
     */
    private ParcelFileDescriptor dup(MemoryFile memoryFile)
    {
        if (memoryFile != null)
        {
            try
            {
                Method method = MemoryFile.class.getDeclaredMethod("getFileDescriptor");
                FileDescriptor fd = (FileDescriptor) method.invoke(memoryFile);
                return ParcelFileDescriptor.dup(fd);
            } catch (Exception e)
            {
                e.printStackTrace();
            }
        }
        return null;
    }

    private final native long nativeCreate();

    private final native void nativeDestroy(final long ptr);

    private final native boolean nativeIsConnected(final long ptr);

    private final native boolean nativeConnect(final long ptr, String filename, int fd);

    private final native void nativeRelease(final long ptr);

    private final native boolean nativeFormat(final long ptr, int pixelformat, int fps, int width, int height);

    private final native int nativeGetPixelformat(final long ptr);

    private final native int nativeGetFps(final long ptr);

    private final native int nativeGetWidth(final long ptr);

    private final native int nativeGetHeight(final long ptr);

    private final native void nativeStartCapture(final long ptr);

    private final native void nativeStopCapture(final long ptr);

    private final native boolean nativeIsCapture(final long ptr);

    private final native void nativeAddPreviewDisplay(final long ptr, int id, Surface surface);

    private final native void nativeRemovePreviewDisplay(final long ptr, int id);

    private final native void nativeClearPreviewDisplay(final long ptr);

    private final native int nativeCountPreviewDisplay(final long ptr);

    private final native void nativeStartPreview(final long ptr);

    private final native void nativeStopPreview(final long ptr);

    private final native boolean nativeIsPreview(final long ptr);

    private final native void nativeSetFrameCallback(final long mNativePtr, IFrameCallback frameCallback);

    private final native MemoryFile nativeCaptureOpenShared(final long mNativePtr, int length, ISharedCallback sharedCallback);

    private final native void nativeCaptureCloseShared(final long mNativePtr);

    private final native MemoryFile nativePreviewOpenShared(final long mNativePtr, int length, ISharedCallback sharedCallback);

    private final native void nativePreviewCloseShared(final long mNativePtr);
}
