package com.cit.uvccamera.app.service;

import android.content.Intent;
import android.hardware.usb.UsbDevice;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.annotation.Nullable;
import android.util.Log;
import android.util.SparseArray;
import android.view.Surface;

import com.cit.uvccamera.USBMonitor;
import com.cit.uvccamera.UsbDeviceManager;
import com.cit.uvccamera.aidl.ICoreService;
import com.cit.uvccamera.aidl.ICoreServiceCallback;
import com.cit.uvccamera.aidl.ISharedServiceCallback;
import com.cit.uvccamera.app.base.BaseService;

import java.io.File;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

public class CoreService extends BaseService
{
    private static final boolean DEBUG = true;
    private static final String TAG = "CoreService";

    @Nullable
    @Override
    public IBinder onBind(Intent intent)
    {
        if (DEBUG) Log.d(TAG, "onBind: " + intent);
        if ((ICoreService.class.getName()).equals(intent.getAction()))
        {
            Log.v(TAG, "return mCoreService");
            return mCoreService;
        }
        return null;
    }

    @Override
    public void onRebind(Intent intent)
    {
        super.onRebind(intent);
        if (DEBUG) Log.d(TAG, "onRebind: " + intent);
    }

    @Override
    public boolean onUnbind(Intent intent)
    {
        if (DEBUG) Log.d(TAG, "onUnbind: " + intent);
        return super.onUnbind(intent);
    }

    private List<UsbDevice> mUsbDevices = new ArrayList<UsbDevice>();
    private Set<UsbDevice> mRequestPermission = new HashSet<UsbDevice>();
    private Set<String> mVideos = new HashSet<String>();


    @Override
    public void onCreate()
    {
        super.onCreate();
        loadUsbDevices();
        if (mUSBMonitor == null)
        {
            mUSBMonitor = new USBMonitor(this, mOnDeviceConnectListener);
            mUSBMonitor.register();
        }
    }

    @Override
    public void onDestroy()
    {
        if (mUSBMonitor != null)
        {
            mUSBMonitor.unregister();
            mUSBMonitor = null;
        }
        clearCoreServer();
        mVideos.clear();
        mRequestPermission.clear();
        mUsbDevices.clear();
        super.onDestroy();
    }

    /**
     *
     */
    private void loadUsbDevices()
    {
        mUsbDevices = UsbDeviceManager.loadUsbDevices(this);
//        mUsbDevices = UsbDeviceManager.listFromDevBus(mContext);
    }

    private void tryRequestPermission(UsbDevice device)
    {
        if (!mRequestPermission.contains(device))
        {
            mRequestPermission.add(device);
            mUSBMonitor.requestPermission(device, true);
        }
    }

    private void outRequestPermission(UsbDevice device)
    {
        if (mRequestPermission.contains(device))
        {
            mRequestPermission.remove(device);
        }
    }


    private USBMonitor mUSBMonitor;
    /**
     * 请求设备设备权限回调
     */
    private USBMonitor.OnDeviceConnectListener mOnDeviceConnectListener = new USBMonitor.OnDeviceConnectListener()
    {

        @Override
        public void onAttach(UsbDevice device)
        {
            Log.d(TAG, "onAttach: ");
            if (device != null &&
                    device.getDeviceClass() == 239 && device.getDeviceSubclass() == 2)
            {
                loadUsbDevices();
            }
        }

        @Override
        public void onDettach(UsbDevice device)
        {
            Log.d(TAG, "onDettach: ");
            outRequestPermission(device);
            if (device != null &&
                    device.getDeviceClass() == 239 && device.getDeviceSubclass() == 2)
            {
                removeCoreServer(device.getDeviceName().hashCode());
                checkVideos();
                loadUsbDevices();
            }
        }

        @Override
        public void onConnect(UsbDevice device, USBMonitor.UsbControlBlock ctrlBlock, boolean createNew)
        {
            Log.d(TAG, "onConnect");
            outRequestPermission(device);
            CoreServer server = getCoreServer(device.getDeviceName().hashCode());
            if (server != null)
            {
                server.connect(device.getDeviceName(), ctrlBlock.getFileDescriptor());
            }
        }

        @Override
        public void onDisconnect(UsbDevice device, USBMonitor.UsbControlBlock ctrlBlock)
        {
            Log.d(TAG, "onDisconnect");
            outRequestPermission(device);
        }

        @Override
        public void onCancel(UsbDevice device)
        {
            Log.d(TAG, "onCancel");
            outRequestPermission(device);
        }

    };

    //====================================================================================================

    private static final Object sServiceSync = new Object();
    private static final SparseArray<CoreServer> sCoreServers = new SparseArray<CoreServer>();

    private CoreServer getCoreServer(final int id)
    {
        CoreServer server;
        synchronized (sServiceSync)
        {
            server = sCoreServers.get(id);
        }
        return server;
    }

    private void removeCoreServer(final int id)
    {
        CoreServer server;
        synchronized (sServiceSync)
        {
            server = sCoreServers.get(id);
            if (server != null)
            {
                server.release();
                sCoreServers.remove(id);
            }
        }
    }

    private void checkVideos()
    {
        Iterator<String> iterator = mVideos.iterator();
        while (iterator.hasNext())
        {
            String name = iterator.next();
            File file = new File(name);
            if (!file.exists())
            {
                removeCoreServer(name.hashCode());
                iterator.remove();
            }
        }
    }

    private void clearCoreServer()
    {
        CoreServer server;
        synchronized (sServiceSync)
        {
            int n = sCoreServers.size();
            for (int i = 0; i < n; i++)
            {
                server = sCoreServers.valueAt(i);
                server.release();
            }
            sCoreServers.clear();
        }
    }

    private CoreServer generateCoreServer(int id)
    {
        CoreServer server;
        synchronized (sServiceSync)
        {
            server = sCoreServers.get(id);
            if (server == null)
            {
                server = CoreServer.createServer(this, id);
                sCoreServers.append(id, server);
            }
        }
        return server;
    }

    private CoreServer selectCoreServer(int id)
    {
        CoreServer server = null;
        if (!mUsbDevices.isEmpty())
        {
            UsbDevice usbDevice;
            if (id == 0)
            {
                usbDevice = mUsbDevices.get(0);
                id = usbDevice.getDeviceName().hashCode();
                server = generateCoreServer(id);
            } else
            {
                int size = mUsbDevices.size();
                for (int i = 0; i < size; i++)
                {
                    usbDevice = mUsbDevices.get(i);
                    if (id == usbDevice.getDeviceName().hashCode())
                    {
                        server = generateCoreServer(id);
                        break;
                    }
                }
            }
        }
        return server;
    }

    private UsbDevice selectUsbDevice(int id)
    {
        UsbDevice device = null;
        for (UsbDevice temp : mUsbDevices)
        {
            if (id == temp.getDeviceName().hashCode())
            {
                device = temp;
                break;
            }
        }
        return device;
    }

    /**
     *
     */
    private final ICoreService.Stub mCoreService = new ICoreService.Stub()
    {

        @Override
        public List<UsbDevice> listUsbDevice() throws RemoteException
        {
            if (DEBUG) Log.d(TAG, "listUsbDevice: ");
            loadUsbDevices();
            return mUsbDevices;
        }

        @Override
        public void select(int id, ICoreServiceCallback callback) throws RemoteException
        {
            if (DEBUG) Log.d(TAG, "select: ");
            CoreServer server = selectCoreServer(id);
            if (server != null)
            {
                boolean flag = server.registerCallback(callback);
                if (flag && !server.isConnected())
                {
                    UsbDevice device = selectUsbDevice(server.getServerId());
                    if (device != null)
                    {
                        if (device.getDeviceClass() == 239 && device.getDeviceSubclass() == 2)
                        {
                            tryRequestPermission(device);
                        } else
                        {
                            String name = device.getDeviceName();
                            mVideos.add(name);
                            server.connect(name, 0);
                        }
                    }
                }
            }
        }

        @Override
        public void unselect(int id, ICoreServiceCallback callback) throws RemoteException
        {
            if (DEBUG) Log.d(TAG, "unselect: ");
            CoreServer server = getCoreServer(id);
            if (server != null)
            {
                server.unregisterCallback(callback);
                if (server.isEmptyCoreServiceCallbacks())
                {
                    synchronized (sServiceSync)
                    {
                        server.release();
                        sCoreServers.remove(id);
                    }
                }
            }
        }

        @Override
        public void format(int id, int pixelformat, int fps, int width, int height) throws RemoteException
        {
            if (DEBUG) Log.d(TAG, "format: ");
            CoreServer server = getCoreServer(id);
            if (server != null)
                server.format(pixelformat, fps, width, height);
        }

        @Override
        public void addSurface(int id, int surface_id, Surface surface) throws RemoteException
        {
            if (DEBUG) Log.d(TAG, "addSurface: ");
            CoreServer server = getCoreServer(id);
            if (server != null)
            {
                int pid = getCallingPid();
                server.addSurface(pid, surface_id, surface);
            }
        }

        @Override
        public void removeSurface(int id, int surface_id) throws RemoteException
        {
            if (DEBUG) Log.d(TAG, "removeSurface: ");
            CoreServer server = getCoreServer(id);
            if (server != null)
            {
                int pid = getCallingPid();
                server.removeSurface(pid, surface_id);
            }
        }

        @Override
        public void addCaptureCallback(int id, ISharedServiceCallback callback) throws RemoteException
        {
            if (DEBUG) Log.d(TAG, "addCaptureCallback: ");
            CoreServer server = getCoreServer(id);
            if (server != null)
                server.registerCaptureCallback(callback);
        }

        @Override
        public void removeCaptureCallback(int id, ISharedServiceCallback callback) throws RemoteException
        {
            if (DEBUG) Log.d(TAG, "removeCaptureCallback: ");
            CoreServer server = getCoreServer(id);
            if (server != null)
                server.unregisterCaptureCallback(callback);
        }

        @Override
        public void addPreviewCallback(int id, ISharedServiceCallback callback) throws RemoteException
        {
            if (DEBUG) Log.d(TAG, "addPreviewCallback: ");
            CoreServer server = getCoreServer(id);
            if (server != null)
                server.registerPreviewCallback(callback);
        }

        @Override
        public void removePreviewCallback(int id, ISharedServiceCallback callback) throws RemoteException
        {
            if (DEBUG) Log.d(TAG, "removePreviewCallback: ");
            CoreServer server = getCoreServer(id);
            if (server != null)
                server.unregisterPreviewCallback(callback);
        }
    };

}
