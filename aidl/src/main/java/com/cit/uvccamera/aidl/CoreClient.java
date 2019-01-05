package com.cit.uvccamera.aidl;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.hardware.usb.UsbDevice;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.MemoryFile;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;
import android.view.Surface;

import java.lang.ref.WeakReference;
import java.util.List;

/**
 * @see ICore
 */
public class CoreClient implements ICore
{
    private static final String PACKAGE_NAME = "com.cit.uvcamera.app";
    private static final boolean DEBUG = true;
    private static final String TAG = CoreClient.class.getSimpleName();

    private boolean isRelease;
    private final WeakReference<Context> mWeakContext;
    private final WeakReference<CoreHandler> mWeakHandler;

    private final Object mServiceSync = new Object();
    private ICoreService mService;
    private ICoreCallback mListener;
    //
    private ISharedCallback mCaptureCallback;
    private ISharedCallback mPreviewCallback;

    public CoreClient(final Context context, final ICoreCallback listener)
    {
        mWeakContext = new WeakReference<Context>(context);
        mListener = listener;
        mWeakHandler = new WeakReference<CoreHandler>(CoreHandler.createHandler(this));
        doBindService();
    }

    @Override
    protected void finalize() throws Throwable
    {
        if (DEBUG) Log.v(TAG, "finalize:");
        super.finalize();
    }

    /**
     * @param context
     * @param implicitIntent
     * @return
     */
    private Intent getExplicitIntent(Context context, Intent implicitIntent)
    {
        // Retrieve all services that can match the given intent
        PackageManager pm = context.getPackageManager();
        List<ResolveInfo> resolveInfo = pm.queryIntentServices(implicitIntent, 0);
        // Make sure only one match was found
        if (resolveInfo == null || resolveInfo.size() != 1)
        {
            // null return implicitIntent
            return implicitIntent;
        }
        // Get component info and create ComponentName
        ResolveInfo serviceInfo = resolveInfo.get(0);
        String packageName = serviceInfo.serviceInfo.packageName;
        String className = serviceInfo.serviceInfo.name;
        ComponentName component = new ComponentName(packageName, className);
        // Create a new intent. Use the old one for extras and such reuse
        Intent explicitIntent = new Intent(implicitIntent);
        // Set the component to be explicit
        explicitIntent.setComponent(component);
        return explicitIntent;
    }


    /**
     * 绑定服务
     *
     * @return
     */
    private void doBindService()
    {
        addHello(5000);
        //if (DEBUG) Log.v(TAG, "doBindService:");
        synchronized (mServiceSync)
        {
            if (mService == null)
            {
                final Context context = mWeakContext.get();
                if (context != null)
                {
                    try
                    {
                        final Intent intent = new Intent(ICoreService.class.getName());
                        final Intent service = getExplicitIntent(context, intent);
                        service.setPackage(PACKAGE_NAME);
                        context.bindService(service, mServiceConnection, Context.BIND_AUTO_CREATE);
                    } catch (Exception e)
                    {
                        Log.w(TAG, "doBindService:", e);
                    }
                }
            }
        }
    }

    /**
     * 解绑服务
     */
    private void doUnBindService()
    {
        removeHello();
        //if (DEBUG) Log.v(TAG, "doUnBindService:");
        synchronized (mServiceSync)
        {
            if (mService != null)
            {
                final Context context = mWeakContext.get();
                if (context != null)
                {
                    try
                    {
                        context.unbindService(mServiceConnection);
                    } catch (Exception e)
                    {
                        Log.w(TAG, "doUnBindService:", e);
                    }
                }
                mService = null;
            }
        }
    }

    /**
     * 绑定服务监听
     */
    private final ServiceConnection mServiceConnection = new ServiceConnection()
    {
        /**
         * 与服务建立连接
         */
        @Override
        public void onServiceConnected(final ComponentName name, final IBinder service)
        {
            if (DEBUG) Log.v(TAG, "onServiceConnected:name=" + name);
            synchronized (mServiceSync)
            {
                mService = ICoreService.Stub.asInterface(service);
                mServiceSync.notifyAll();
            }
            if (mListener != null)
            {
                mListener.onServiceConnected();
            }
            // hello
            removeHello();
        }

        /**
         * 与服务断开连接
         */
        @Override
        public void onServiceDisconnected(final ComponentName name)
        {
            if (DEBUG) Log.v(TAG, "onServiceDisconnected:name=" + name);
            synchronized (mServiceSync)
            {
                mService = null;
                mServiceSync.notifyAll();
            }
            if (mListener != null)
            {
                mListener.onServiceDisconnected();
            }
            // client没有释放资源，1秒后执行重新连接
            if (!isRelease)
            {
                addHello(1000);
            }
        }
    };

    private ICoreService getService()
    {
        if (mService == null)
        {
            synchronized (mServiceSync)
            {
                if (mService == null)
                {
                    try
                    {
                        mServiceSync.wait();
                    } catch (final InterruptedException e)
                    {
                        e.printStackTrace();
                    }
                }
            }
        }
        return mService;
    }

    private void addHello(long delayMillis)
    {
        final CoreHandler handler = mWeakHandler.get();
        if (null != handler)
        {
            handler.removeMessages(MSG_HELLO);
            handler.sendEmptyMessageDelayed(MSG_HELLO, delayMillis);
        }
    }

    private void removeHello()
    {
        final CoreHandler handler = mWeakHandler.get();
        if (null != handler)
        {
            handler.removeMessages(MSG_HELLO);
        }
    }

    private void sayHello()
    {
        if (DEBUG) Log.v(TAG, "sayHello:");
        if (!isRelease && mService == null)
        {
            doBindService();
        }
    }

    private void sayBye()
    {
        if (DEBUG) Log.v(TAG, "sayBye:");
        if (isRelease && mService != null)
        {
            doUnBindService();
        }
    }

    @Override
    public List<UsbDevice> listUsbDevice()
    {
        final CoreHandler handler = mWeakHandler.get();
        if (handler != null)
            return handler.listUsbDevice();
        return null;
    }

    @Override
    public void release()
    {
        if (isRelease)
        {
            if (DEBUG) Log.w(TAG, "already release");
            return;
        }
        isRelease = true;
        final CoreHandler handler = mWeakHandler.get();
        if (handler != null)
        {
            handler.removeCallbacksAndMessages(null);
            handler.sendEmptyMessage(MSG_RELEASE);
        }
    }

    @Override
    public void select(int id)
    {
        final CoreHandler handler = mWeakHandler.get();
        if (handler != null)
            handler.sendMessage(handler.obtainMessage(MSG_SELECT, id, 0));
    }

    @Override
    public void unselect()
    {
        final CoreHandler handler = mWeakHandler.get();
        if (handler != null)
            handler.sendEmptyMessage(MSG_UNSELECT);
    }

    @Override
    public void format(int pixelformat, int fps, int width, int height)
    {
        final CoreHandler handler = mWeakHandler.get();
        if (handler != null)
            handler.sendMessage(handler.obtainMessage(MSG_FORMAT, new int[]{pixelformat, fps, width, height}));
    }

    @Override
    public void addSurface(Surface surface)
    {
        final CoreHandler handler = mWeakHandler.get();
        if (handler != null)
            handler.sendMessage(handler.obtainMessage(MSG_ADD_SURFACE, surface.hashCode(), 0, surface));
    }

    @Override
    public void removeSurface(Surface surface)
    {
        final CoreHandler handler = mWeakHandler.get();
        if (handler != null)
            handler.sendMessage(handler.obtainMessage(MSG_REMOVE_SURFACE, surface.hashCode(), 0));
    }

    @Override
    public void setCaptureCallback(ISharedCallback callback)
    {
        final CoreHandler handler = mWeakHandler.get();
        if (handler != null)
        {
            mCaptureCallback = callback;
            handler.sendEmptyMessage(mCaptureCallback == null ? MSG_REMOVE_CAPTURE_CALLBACK : MSG_ADD_CAPTURE_CALLBACK);
        }
    }

    @Override
    public void setPreviewCallback(ISharedCallback callback)
    {
        final CoreHandler handler = mWeakHandler.get();
        if (handler != null)
        {
            mPreviewCallback = callback;
            handler.sendEmptyMessage(mPreviewCallback == null ? MSG_REMOVE_PREVIEW_CALLBACK : MSG_ADD_PREVIEW_CALLBACK);
        }
    }

    private static final int MSG_HELLO = -1;//保活
    private static final int MSG_RELEASE = 0;//释放
    private static final int MSG_SELECT = 1;
    private static final int MSG_UNSELECT = 2;
    private static final int MSG_FORMAT = 3;
    private static final int MSG_ADD_SURFACE = 4;
    private static final int MSG_REMOVE_SURFACE = 5;
    private static final int MSG_ADD_CAPTURE_CALLBACK = 6;
    private static final int MSG_REMOVE_CAPTURE_CALLBACK = 7;
    private static final int MSG_ADD_PREVIEW_CALLBACK = 8;
    private static final int MSG_REMOVE_PREVIEW_CALLBACK = 9;

    /**
     *
     */
    private static final class CoreHandler extends Handler
    {
        private CoreTask mTask;

        private CoreHandler(final CoreTask task)
        {
            mTask = task;
        }

        private static CoreHandler createHandler(final CoreClient parent)
        {
            final CoreTask runnable = new CoreTask(parent);
            new Thread(runnable).start();
            return runnable.getHandler();
        }

        @Override
        public void handleMessage(Message msg)
        {
            if (DEBUG) Log.d(TAG, "handleMessage: " + msg.what);
            switch (msg.what)
            {
                case MSG_HELLO:
                    mTask.handleHello();
                    break;
                case MSG_RELEASE:
                    mTask.handleRemoveCaptureCallback();
                    mTask.handleUnselect();
                    mTask.handleRelease();
                    mTask = null;
                    Looper.myLooper().quit();
                    break;
                case MSG_SELECT:
                    mTask.handleSelect(msg.arg1);
                    break;
                case MSG_UNSELECT:
                    mTask.handleUnselect();
                    break;
                case MSG_FORMAT:
                    int[] params = (int[]) msg.obj;
                    mTask.handleFormat(params[0], params[1], params[2], params[3]);
                    break;
                case MSG_ADD_SURFACE:
                    mTask.handleAddSurface(msg.arg1, (Surface) msg.obj);
                    break;
                case MSG_REMOVE_SURFACE:
                    mTask.handleRemoveSurface(msg.arg1);
                    break;
                case MSG_ADD_CAPTURE_CALLBACK:
                    mTask.handleAddCaptureCallback();
                    break;
                case MSG_REMOVE_CAPTURE_CALLBACK:
                    mTask.handleRemoveCaptureCallback();
                    break;
                case MSG_ADD_PREVIEW_CALLBACK:
                    mTask.handleAddPreviewCallback();
                    break;
                case MSG_REMOVE_PREVIEW_CALLBACK:
                    mTask.handleRemovePreviewCallback();
                    break;
            }
        }

        //==================================================
        //
        //==================================================
        private List<UsbDevice> listUsbDevice()
        {
            final ICoreService service = mTask.mParent.mService;
            if (service != null)
            {
                try
                {
                    return service.listUsbDevice();
                } catch (RemoteException e)
                {
                    if (DEBUG) Log.w(TAG, e);
                }
            }
            return null;
        }

        /**
         *
         */
        private static final class CoreTask extends ICoreServiceCallback.Stub implements Runnable
        {
            private static final String TAG = CoreTask.class.getSimpleName();
            private final Object mSync = new Object();
            private CoreClient mParent;
            private CoreHandler mHandler;
            private int mId;

            private CoreTask(final CoreClient parent)
            {
                mParent = parent;
            }

            private CoreHandler getHandler()
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
                if (DEBUG) Log.v(TAG, "finalize:");
                super.finalize();
            }

            @Override
            public void run()
            {
                if (DEBUG) Log.v(TAG, "run:");
                Looper.prepare();
                synchronized (mSync)
                {
                    mHandler = new CoreHandler(this);
                    mSync.notifyAll();
                }
                Looper.loop();
                if (DEBUG) Log.v(TAG, "run:finishing");
                synchronized (mSync)
                {
                    mHandler = null;
                    mParent = null;
                    mSync.notifyAll();
                }
            }

            //==================================================
            //+++handleMessage
            //==================================================
            private void handleHello()
            {
                // sayHello
                mParent.sayHello();
            }

            private void handleRelease()
            {
                // sayBye
                mParent.sayBye();
            }

            private void handleSelect(int id)
            {
                final ICoreService service = mParent.getService();
                if (service != null)
                {
                    try
                    {
                        if (id != mId)
                        {
                            service.removePreviewCallback(mId, mPreviewSharedCallback);
                            service.removeCaptureCallback(mId, mCaptureSharedCallback);
                            service.unselect(mId, this);
                        }
                        service.select(id, this);
                    } catch (final RemoteException e)
                    {
                        if (DEBUG) Log.w(TAG, e);
                    }
                }
            }

            private void handleUnselect()
            {
                final ICoreService service = mParent.getService();
                if (service != null)
                {
                    try
                    {
                        service.removePreviewCallback(mId, mPreviewSharedCallback);
                        service.removeCaptureCallback(mId, mCaptureSharedCallback);
                        service.unselect(mId, this);
                    } catch (final RemoteException e)
                    {
                        if (DEBUG) Log.w(TAG, e);
                    }
                }
            }

            private void handleFormat(int pixelformat, int fps, int width, int height)
            {
                final ICoreService service = mParent.getService();
                if (service != null)
                {
                    try
                    {
                        service.format(mId, pixelformat, fps, width, height);
                    } catch (final RemoteException e)
                    {
                        if (DEBUG) Log.w(TAG, e);
                    }
                }
            }

            private void handleAddSurface(int surface_id, Surface surface)
            {
                final ICoreService service = mParent.getService();
                if (service != null)
                {
                    try
                    {
                        service.addSurface(mId, surface_id, surface);
                    } catch (final RemoteException e)
                    {
                        if (DEBUG) Log.w(TAG, e);
                    }
                }
            }

            private void handleRemoveSurface(int surface_id)
            {
                final ICoreService service = mParent.getService();
                if (service != null)
                {
                    try
                    {
                        service.removeSurface(mId, surface_id);
                    } catch (final RemoteException e)
                    {
                        if (DEBUG) Log.w(TAG, e);
                    }
                }
            }

            private void handleAddCaptureCallback()
            {
                final ICoreService service = mParent.getService();
                if (service != null)
                {
                    try
                    {
                        service.addCaptureCallback(mId, mCaptureSharedCallback);
                    } catch (final RemoteException e)
                    {
                        if (DEBUG) Log.w(TAG, e);
                    }
                }
            }

            private void handleRemoveCaptureCallback()
            {
                final ICoreService service = mParent.getService();
                if (service != null)
                {
                    try
                    {
                        service.removeCaptureCallback(mId, mCaptureSharedCallback);
                    } catch (final RemoteException e)
                    {
                        if (DEBUG) Log.w(TAG, e);
                    }
                }
            }

            private void handleAddPreviewCallback()
            {
                final ICoreService service = mParent.getService();
                if (service != null)
                {
                    try
                    {
                        service.addPreviewCallback(mId, mPreviewSharedCallback);
                    } catch (final RemoteException e)
                    {
                        if (DEBUG) Log.w(TAG, e);
                    }
                }
            }

            private void handleRemovePreviewCallback()
            {
                final ICoreService service = mParent.getService();
                if (service != null)
                {
                    try
                    {
                        service.removePreviewCallback(mId, mPreviewSharedCallback);
                    } catch (final RemoteException e)
                    {
                        if (DEBUG) Log.w(TAG, e);
                    }
                }
            }
            //==================================================
            //---handleMessage
            //==================================================


            //==================================================
            //+++ICoreServiceCallback
            //==================================================
            @Override
            public void onSelected(int id) throws RemoteException
            {
                if (DEBUG) Log.d(TAG, "onSelected: " + id);
                mId = id;
                if (mParent != null)
                    if (mParent.mListener != null)
                        mParent.mListener.onSelected();
            }

            @Override
            public void onUnselected() throws RemoteException
            {
                mId = 0;
                if (mParent != null)
                    if (mParent.mListener != null)
                        mParent.mListener.onUnselected();
            }

            @Override
            public void onConnected() throws RemoteException
            {
                if (mParent != null)
                    if (mParent.mListener != null)
                        mParent.mListener.onConnected();
            }

            @Override
            public void onDisConnected() throws RemoteException
            {
                if (mParent != null)
                    if (mParent.mListener != null)
                        mParent.mListener.onDisConnected();
            }

            @Override
            public void onFormat(int pixelformat, int fps, int width, int height) throws RemoteException
            {
                if (mParent != null)
                    if (mParent.mListener != null)
                        mParent.mListener.onFormat(pixelformat, fps, width, height);
            }

            //==================================================
            //---ICoreServiceCallback
            //==================================================


            //==================================================
            //+++ISharedServiceCallback
            //==================================================
            private ISharedServiceCallback.Stub mCaptureSharedCallback = new ISharedServiceCallback.Stub()
            {
                private Shared mShared;

                @Override
                public void onAdded(ParcelFileDescriptor pfd, int length) throws RemoteException
                {
                    if (DEBUG) Log.d(TAG, "mCaptureSharedCallback#onAdded");
                    if (mShared == null)
                    {
                        mShared = new Shared(pfd, length);
                    }
                }

                @Override
                public void onRemoved() throws RemoteException
                {
                    if (DEBUG) Log.d(TAG, "mCaptureSharedCallback#onRemoved");
                    if (mShared != null)
                    {
                        mShared.recycle();
                        mShared = null;
                    }
                }

                @Override
                public void onUpdated(int available) throws RemoteException
                {
                    //if (DEBUG) Log.d(TAG, "mCaptureSharedCallback#onUpdated");
                    if (mShared != null)
                    {
                        int n = mShared.read(available);
                        if (n > 0)
                        {
                            if (mParent != null)
                                if (mParent.mCaptureCallback != null)
                                    mParent.mCaptureCallback.onUpdated(mShared.body, available);
                        }
                    }
                }
            };

            private ISharedServiceCallback.Stub mPreviewSharedCallback = new ISharedServiceCallback.Stub()
            {
                private Shared mShared;

                @Override
                public void onAdded(ParcelFileDescriptor pfd, int length) throws RemoteException
                {
                    if (DEBUG) Log.d(TAG, "mPreviewSharedCallback#onAdded");
                    if (mShared == null)
                    {
                        mShared = new Shared(pfd, length);
                    }
                }

                @Override
                public void onRemoved() throws RemoteException
                {
                    if (DEBUG) Log.d(TAG, "mPreviewSharedCallback#onRemoved");
                    if (mShared != null)
                    {
                        mShared.recycle();
                        mShared = null;
                    }
                }

                @Override
                public void onUpdated(int available) throws RemoteException
                {
                    //if (DEBUG) Log.d(TAG, "mPreviewSharedCallback#onUpdated");
                    if (mShared != null)
                    {
                        int n = mShared.read(available);
                        if (n > 0)
                        {
                            if (mParent != null)
                                if (mParent.mPreviewCallback != null)
                                    mParent.mPreviewCallback.onUpdated(mShared.body, available);
                        }
                    }
                }
            };
            //==================================================
            //---ISharedServiceCallback
            //==================================================
        }
    }
}
