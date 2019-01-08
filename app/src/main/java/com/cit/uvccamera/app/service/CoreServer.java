package com.cit.uvccamera.app.service;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;
import android.view.Surface;

import com.cit.uvccamera.ISharedCallback;
import com.cit.uvccamera.UVCCamera;
import com.cit.uvccamera.aidl.ICoreServiceCallback;
import com.cit.uvccamera.aidl.ISharedServiceCallback;

import java.lang.ref.WeakReference;

public class CoreServer extends Handler
{

    private static final boolean DEBUG = false;
    private static final String TAG = "CoreServer";

    private boolean isRelease;
    private final int mServerId;
    private final WeakReference<CoreThread> mWeakThread;
    private final RemoteCallbackList<ICoreServiceCallback> mCoreServiceCallbacks = new RemoteCallbackList<ICoreServiceCallback>();
    private final RemoteCallbackList<ISharedServiceCallback> mCaptureSharedCallbacks = new RemoteCallbackList<ISharedServiceCallback>();
    private int mCaptureSharedCallbacksCount;//记录Capture分享个数
    private final RemoteCallbackList<ISharedServiceCallback> mPreviewSharedCallbacks = new RemoteCallbackList<ISharedServiceCallback>();
    private int mPreviewSharedCallbacksCount;//记录Preview分享个数

    /**
     * Constructor
     *
     * @param thread
     */
    private CoreServer(final CoreThread thread, final int serverId)
    {
        if (DEBUG)
        {
            String hashCode = Integer.toHexString(hashCode());
            Log.d(TAG, "Constructor:@" + hashCode);
        }
        mServerId = serverId;
        mWeakThread = new WeakReference<CoreThread>(thread);
    }

    public static CoreServer createServer(final Context context, final int serverId)
    {
        if (DEBUG)
            Log.d(TAG, "createServer:");
        final CoreThread thread = new CoreThread(context, serverId);
        thread.start();
        return thread.getHandler();
    }

    private void killCallbacks()
    {
        mPreviewSharedCallbacks.kill();
        mCaptureSharedCallbacks.kill();
        mCoreServiceCallbacks.kill();
    }

    //==================================================
    //+++ICoreServiceCallback
    //==================================================
    public boolean registerCallback(final ICoreServiceCallback callback)
    {
        if (mCoreServiceCallbacks.register(callback))
        {
            // 注册时，触发一些回调
            try
            {
                callback.onSelected(mServerId);
                sendMessage(obtainMessage(MSG_IS_CONNECTED, callback));
            } catch (RemoteException e)
            {
                Log.w(TAG, e);
            }
            return true;
        }
        return false;
    }

    public boolean unregisterCallback(final ICoreServiceCallback callback)
    {
        if (mCoreServiceCallbacks.unregister(callback))
        {
            // 注销时，触发一些回调
            try
            {
                callback.onUnselected();
            } catch (RemoteException e)
            {
                Log.w(TAG, e);
            }
            return true;
        }
        return false;
    }

    public boolean isEmptyCoreServiceCallbacks()
    {
        return 0 == mCoreServiceCallbacks.getRegisteredCallbackCount();
    }

    private void onConnectedCallback()
    {
        try
        {
            final int m = mCoreServiceCallbacks.beginBroadcast();
            for (int i = 0; i < m; i++)
            {
                try
                {
                    mCoreServiceCallbacks.getBroadcastItem(i).onConnected();
                } catch (final Exception e)
                {
                    e.printStackTrace();
                }
            }
            mCoreServiceCallbacks.finishBroadcast();
        } catch (final Exception e)
        {
            Log.w(TAG, e);
        }
    }

    private void onDisConnectedCallback()
    {
        try
        {
            final int m = mCoreServiceCallbacks.beginBroadcast();
            for (int i = 0; i < m; i++)
            {
                try
                {
                    mCoreServiceCallbacks.getBroadcastItem(i).onDisConnected();
                } catch (final Exception e)
                {
                    e.printStackTrace();
                }
            }
            mCoreServiceCallbacks.finishBroadcast();
        } catch (final Exception e)
        {
            Log.w(TAG, e);
        }
    }

    private void onFormatCallback(int pixelformat, int fps, int width, int height)
    {
        try
        {
            final int m = mCoreServiceCallbacks.beginBroadcast();
            for (int i = 0; i < m; i++)
            {
                try
                {
                    mCoreServiceCallbacks.getBroadcastItem(i).onFormat(pixelformat, fps, width, height);
                } catch (final Exception e)
                {
                    e.printStackTrace();
                }
            }
            mCoreServiceCallbacks.finishBroadcast();
        } catch (final Exception e)
        {
            Log.w(TAG, e);
        }
    }
    //==================================================
    //---ICoreServiceCallback
    //==================================================

    //==================================================
    //+++ISharedServiceCallback Capture
    //==================================================
    public boolean registerCaptureCallback(final ISharedServiceCallback callback)
    {
        if (mCaptureSharedCallbacks.register(callback))
        {
            // 注册时，触发一些回调
            mCaptureSharedCallbacksCount = mCaptureSharedCallbacks.getRegisteredCallbackCount();
            if (mCaptureSharedCallbacksCount > 0)
            {
                removeMessages(MSG_CHECK_CAPTURE_CALLBACK);//检测（避免client没有注销就已经崩溃了）
                sendMessage(obtainMessage(MSG_ADD_CAPTURE_CALLBACK, callback));
                //
                sendEmptyMessageDelayed(MSG_CHECK_CAPTURE_CALLBACK, 1000);
            }
            return true;
        }
        return false;
    }

    public boolean unregisterCaptureCallback(final ISharedServiceCallback callback)
    {
        if (mCaptureSharedCallbacks.unregister(callback))
        {
            // 注销时，触发一些回调
            try
            {
                callback.onRemoved();
            } catch (RemoteException e)
            {
                Log.w(TAG, e);
            }
            mCaptureSharedCallbacksCount = mCaptureSharedCallbacks.getRegisteredCallbackCount();
            if (mCaptureSharedCallbacksCount == 0)
            {
                removeMessages(MSG_CHECK_CAPTURE_CALLBACK);//移除检测
                sendEmptyMessage(MSG_REMOVE_CAPTURE_CALLBACK);
            }
            return true;
        }
        return false;
    }

    private void checkCaptureCallback()
    {
        if (mCaptureSharedCallbacksCount > 0)
        {
            mCaptureSharedCallbacksCount = mCaptureSharedCallbacks.getRegisteredCallbackCount();
            if (mCaptureSharedCallbacksCount == 0)
            {
                Log.w(TAG, "capture shared useless");
                sendEmptyMessage(MSG_REMOVE_CAPTURE_CALLBACK);
            } else
            {
                sendEmptyMessageDelayed(MSG_CHECK_CAPTURE_CALLBACK, 1000);
            }
        }
    }

    private void onUpdatedCaptureCallback(int available)
    {
        try
        {
            final int m = mCaptureSharedCallbacks.beginBroadcast();
            for (int i = 0; i < m; i++)
            {
                try
                {
                    mCaptureSharedCallbacks.getBroadcastItem(i).onUpdated(available);
                } catch (final Exception e)
                {
                    e.printStackTrace();
                }
            }
            mCaptureSharedCallbacks.finishBroadcast();
        } catch (final Exception e)
        {
            Log.w(TAG, e);
        }
    }
    //==================================================
    //---ISharedServiceCallback Capture
    //==================================================


    //==================================================
    //+++ISharedServiceCallback Preview
    //==================================================
    public boolean registerPreviewCallback(final ISharedServiceCallback callback)
    {
        if (mPreviewSharedCallbacks.register(callback))
        {
            // 注册时，触发一些回调
            mPreviewSharedCallbacksCount = mPreviewSharedCallbacks.getRegisteredCallbackCount();
            if (mPreviewSharedCallbacksCount > 0)
            {
                removeMessages(MSG_CHECK_PREVIEW_CALLBACK);//检测（避免client没有注销就已经崩溃了）
                sendMessage(obtainMessage(MSG_ADD_PREVIEW_CALLBACK, callback));
                //
                sendEmptyMessageDelayed(MSG_CHECK_PREVIEW_CALLBACK, 1000);
            }

            return true;
        }
        return false;
    }

    public boolean unregisterPreviewCallback(final ISharedServiceCallback callback)
    {
        if (mPreviewSharedCallbacks.unregister(callback))
        {
            // 注销时，触发一些回调
            try
            {
                callback.onRemoved();
            } catch (RemoteException e)
            {
                Log.w(TAG, e);
            }
            mPreviewSharedCallbacksCount = mPreviewSharedCallbacks.getRegisteredCallbackCount();
            if (mPreviewSharedCallbacksCount == 0)
            {
                removeMessages(MSG_CHECK_PREVIEW_CALLBACK);//移除检测
                sendEmptyMessage(MSG_REMOVE_PREVIEW_CALLBACK);
            }
            return true;
        }
        return false;
    }

    private void checkPreviewCallback()
    {
        if (mPreviewSharedCallbacksCount > 0)
        {
            mPreviewSharedCallbacksCount = mPreviewSharedCallbacks.getRegisteredCallbackCount();
            if (mPreviewSharedCallbacksCount == 0)
            {
                Log.w(TAG, "preview shared useless");
                sendEmptyMessage(MSG_REMOVE_PREVIEW_CALLBACK);
            } else
            {
                sendEmptyMessageDelayed(MSG_CHECK_PREVIEW_CALLBACK, 1000);
            }
        }
    }

    private void onUpdatedPreviewCallback(int available)
    {
        try
        {
            final int m = mPreviewSharedCallbacks.beginBroadcast();
            for (int i = 0; i < m; i++)
            {
                try
                {
                    mPreviewSharedCallbacks.getBroadcastItem(i).onUpdated(available);
                } catch (final Exception e)
                {
                    e.printStackTrace();
                }
            }
            mPreviewSharedCallbacks.finishBroadcast();
        } catch (final Exception e)
        {
            Log.w(TAG, e);
        }
    }
    //==================================================
    //---ISharedServiceCallback Preview
    //==================================================

    public int getServerId()
    {
        return mServerId;
    }

    public boolean isConnected()
    {
        final CoreThread thread = mWeakThread.get();
        if (thread != null)
            return thread.isConnected();
        return false;
    }

    public void release()
    {
        if (!isRelease)
        {
            isRelease = true;
            removeCallbacksAndMessages(null);
            sendEmptyMessage(MSG_RELEASE);
        }
    }

    public void connect(String name, int fd)
    {
        sendMessage(obtainMessage(MSG_CONNECT, fd, 0, name));
    }

    public void format(int pixelformat, int fps, int width, int height)
    {
        sendMessage(obtainMessage(MSG_FORMAT, new int[]{pixelformat, fps, width, height}));
    }

    public void addSurface(int pid, int sid, Surface surface)
    {
        sendMessage(obtainMessage(MSG_ADD_SURFACE, sid, 0, surface));
    }

    public void removeSurface(int pid, int sid)
    {
        sendMessage(obtainMessage(MSG_REMOVE_SURFACE, sid, 0));
    }

    //==================================================
    //+++handleMessage
    //==================================================

    private static final int MSG_RELEASE = 0;//释放
    private static final int MSG_CONNECT = 1;
    private static final int MSG_IS_CONNECTED = 2;
    private static final int MSG_FORMAT = 3;
    private static final int MSG_ADD_SURFACE = 4;
    private static final int MSG_REMOVE_SURFACE = 5;
    private static final int MSG_ADD_CAPTURE_CALLBACK = 6;
    private static final int MSG_REMOVE_CAPTURE_CALLBACK = 7;
    private static final int MSG_CHECK_CAPTURE_CALLBACK = 8;
    private static final int MSG_ADD_PREVIEW_CALLBACK = 9;
    private static final int MSG_REMOVE_PREVIEW_CALLBACK = 10;
    private static final int MSG_CHECK_PREVIEW_CALLBACK = 11;

    @Override
    public void handleMessage(Message msg)
    {
        if (DEBUG) Log.d(TAG, "handleMessage: " + msg.what);
        final CoreThread thread = mWeakThread.get();
        if (null == thread)
            return;
        switch (msg.what)
        {
            case MSG_RELEASE:
                thread.handleRelease();
            case MSG_CONNECT:
                thread.handleConnect((String) msg.obj, msg.arg1);
                break;
            case MSG_IS_CONNECTED:
                thread.handleIsConnect((ICoreServiceCallback) msg.obj);
                break;
            case MSG_FORMAT:
                int[] params = (int[]) msg.obj;
                thread.handleFormat(params[0], params[1], params[2], params[3]);
                break;
            case MSG_ADD_SURFACE:
                thread.handleAddSurface(msg.arg1, (Surface) msg.obj);
                break;
            case MSG_REMOVE_SURFACE:
                thread.handleRemoveSurface(msg.arg1);
                break;
            case MSG_ADD_CAPTURE_CALLBACK:
                thread.handleAddCaptureCallback((ISharedServiceCallback) msg.obj);
                break;
            case MSG_REMOVE_CAPTURE_CALLBACK:
                thread.handleRemoveCaptureCallback();
                break;
            case MSG_CHECK_CAPTURE_CALLBACK:
                thread.handleCheckCaptureCallback();
                break;
            case MSG_ADD_PREVIEW_CALLBACK:
                thread.handleAddPreviewCallback((ISharedServiceCallback) msg.obj);
                break;
            case MSG_REMOVE_PREVIEW_CALLBACK:
                thread.handleRemovePreviewCallback();
                break;
            case MSG_CHECK_PREVIEW_CALLBACK:
                thread.handleCheckPreviewCallback();
                break;
        }
    }

    /**
     *
     */
    private static final class CoreThread extends Thread
    {
        private static final String TAG = CoreThread.class.getSimpleName();
        private final Object mSync = new Object();
        private final WeakReference<Context> mWeakContext;
        private CoreServer mHandler;
        private int mServerId;
        private UVCCamera mUVCCamera;

        private CoreThread(final Context context, final int serverId)
        {
            super(TAG);
            if (DEBUG)
                Log.d(TAG, "Constructor:");
            this.mWeakContext = new WeakReference<Context>(context);
            this.mServerId = serverId;
            this.mUVCCamera = new UVCCamera();
        }

        public CoreServer getHandler()
        {
            if (mHandler == null)
            {
                synchronized (mSync)
                {
                    if (mHandler == null)
                    {
                        try
                        {
                            mSync.wait();
                        } catch (final InterruptedException e)
                        {
                            e.printStackTrace();
                        }
                    }
                }
            }
            return mHandler;
        }

        @Override
        protected void finalize() throws Throwable
        {
            Log.i(TAG, "finalize");
            super.finalize();
        }

        @Override
        public void run()
        {
            if (DEBUG)
                Log.d(TAG, "run:");
            Looper.prepare();
            synchronized (mSync)
            {
                mHandler = new CoreServer(this, mServerId);
                mSync.notifyAll();
            }
            Looper.loop();
            synchronized (mSync)
            {
                mHandler = null;
                mSync.notifyAll();
            }
            if (DEBUG)
                Log.d(TAG, "run:finished");
        }


        //==================================================
        //+++
        //==================================================
        private ISharedCallback mCaptureShared = new ISharedCallback()
        {
            @Override
            public void onShared(int available)
            {
                mHandler.onUpdatedCaptureCallback(available);
            }
        };

        private ISharedCallback mPreviewShared = new ISharedCallback()
        {
            @Override
            public void onShared(int available)
            {
                mHandler.onUpdatedPreviewCallback(available);
            }
        };
        //==================================================
        //---
        //==================================================


        //==================================================
        //+++
        //==================================================

        private boolean isConnected()
        {
            if (mUVCCamera != null)
                return mUVCCamera.isConnected();
            return false;
        }

        private void handleRelease()
        {
            synchronized (mSync)
            {
                if (mUVCCamera != null)
                {
                    mUVCCamera.release();
                    mUVCCamera = null;
                }
                mHandler.onDisConnectedCallback();
                mHandler.killCallbacks();
                mSync.notifyAll();
            }
            Looper.myLooper().quit();
        }

        private void handleConnect(String name, int fd)
        {
            synchronized (mSync)
            {
                if (mUVCCamera != null)
                {
                    boolean isConnected = mUVCCamera.isConnected();
                    if (!isConnected)
                    {
                        isConnected = mUVCCamera.connect(name, fd);
                        if (isConnected)
                        {
                            mHandler.onConnectedCallback();
                            int pixelformat = mUVCCamera.getPixelformat();
                            int fps = mUVCCamera.getFps();
                            int width = mUVCCamera.getWidth();
                            int height = mUVCCamera.getHeight();
                            mHandler.onFormatCallback(pixelformat, fps, width, height);
                        }
                    }
                }
            }
        }

        private void handleIsConnect(ICoreServiceCallback callback)
        {
            synchronized (mSync)
            {
                if (mUVCCamera != null)
                {
                    if (mUVCCamera.isConnected())
                    {
                        int pixelformat = mUVCCamera.getPixelformat();
                        int fps = mUVCCamera.getFps();
                        int width = mUVCCamera.getWidth();
                        int height = mUVCCamera.getHeight();
                        try
                        {
                            callback.onConnected();
                            callback.onFormat(pixelformat, fps, width, height);
                        } catch (RemoteException e)
                        {
                            Log.w(TAG, e);
                        }
                    }
                }
            }
        }

        private void handleFormat(int pixelformat, int fps, int width, int height)
        {
            synchronized (mSync)
            {
                if (mUVCCamera != null)
                {
                    boolean result = mUVCCamera.format(pixelformat, fps, width, height);
                    if (result)
                    {
                        pixelformat = mUVCCamera.getPixelformat();
                        fps = mUVCCamera.getFps();
                        width = mUVCCamera.getWidth();
                        height = mUVCCamera.getHeight();
                        mHandler.onFormatCallback(pixelformat, fps, width, height);
                    }
                }
            }
        }

        private void handleAddSurface(final int surface_id, final Surface surface)
        {
            synchronized (mSync)
            {
                if (mUVCCamera != null)
                {
                    mUVCCamera.addPreviewDisplay(surface_id, surface);
                }
            }
        }

        private void handleRemoveSurface(final int surface_id)
        {
            synchronized (mSync)
            {
                mUVCCamera.removePreviewDisplay(surface_id);
            }
        }

        private void handleAddCaptureCallback(ISharedServiceCallback callback)
        {
            synchronized (mSync)
            {
                ParcelFileDescriptor pfd = mUVCCamera.captureDupShared(UVCCamera.CAPTURE_SIZE, mCaptureShared);
                if (pfd != null)
                {
                    try
                    {
                        callback.onAdded(pfd, UVCCamera.CAPTURE_SIZE);
                    } catch (RemoteException e)
                    {
                        Log.w(TAG, e);
                    }
                }
            }
        }

        private void handleRemoveCaptureCallback()
        {
            synchronized (mSync)
            {
                mUVCCamera.captureCloseShared();
            }
        }

        private void handleCheckCaptureCallback()
        {
            mHandler.checkCaptureCallback();
        }

        private void handleAddPreviewCallback(ISharedServiceCallback callback)
        {
            synchronized (mSync)
            {
                ParcelFileDescriptor pfd = mUVCCamera.previewDupShared(UVCCamera.PREVIEW_SIZE, mPreviewShared);
                if (pfd != null)
                {
                    try
                    {
                        callback.onAdded(pfd, UVCCamera.PREVIEW_SIZE);
                    } catch (RemoteException e)
                    {
                        Log.w(TAG, e);
                    }
                }
            }
        }

        private void handleRemovePreviewCallback()
        {
            synchronized (mSync)
            {
                mUVCCamera.previewCloseShared();
            }
        }

        private void handleCheckPreviewCallback()
        {
            mHandler.checkPreviewCallback();
        }
        //==================================================
        //---
        //==================================================
    }
}
