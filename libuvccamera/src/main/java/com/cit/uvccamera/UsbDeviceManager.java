package com.cit.uvccamera;

import android.content.Context;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.os.Parcel;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

public class UsbDeviceManager
{

    /**
     * @param context
     * @return
     */
    public static List<UsbDevice> listFromDevBus(Context context)
    {
        List<UsbDevice> usbDevices = new ArrayList<UsbDevice>();
        UsbManager usbManager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        HashMap<String, UsbDevice> deviceList = usbManager.getDeviceList();
        if (deviceList != null)
        {
            List<DeviceFilter> filters = DeviceFilter.getDeviceFilters(context, R.xml.device_filter);
            Iterator<UsbDevice> iterator = deviceList.values().iterator();
            while (iterator.hasNext())
            {
                UsbDevice device = iterator.next();
                for (DeviceFilter filter : filters)
                {
                    if ((filter == null) || (filter.matches(device)))
                    {
                        usbDevices.add(device);
                    }
                }
            }
        }
        return usbDevices;
    }

    /**
     * @return
     */
    public static List<UsbDevice> listFromDevVideo()
    {
        List<UsbDevice> usbDevices = new ArrayList<UsbDevice>();
        File dir = new File("/sys/class/video4linux");
        String[] names = dir.list();
        if (names != null)
        {
            for (String name : names)
            {
                String filename = "/dev/" + name;
                File file = new File(filename);
                if (file.exists())
                {
                    UsbDevice device = createUsbDevice(filename);
                    usbDevices.add(device);
                }
            }
        }
        return usbDevices;
    }


    public static List<UsbDevice> loadUsbDevices(Context context)
    {
        List<UsbDevice> usbDevices = new ArrayList<UsbDevice>();
        usbDevices.addAll(listFromDevBus(context));
        usbDevices.addAll(listFromDevVideo());
        return usbDevices;
    }


    private static UsbDevice createUsbDevice(final String name)
    {
//        String name = in.readString();
//        int vendorId = in.readInt();
//        int productId = in.readInt();
//        int clasz = in.readInt();
//        int subClass = in.readInt();
//        int protocol = in.readInt();
//        String manufacturerName = in.readString();
//        String productName = in.readString();
//        String version = in.readString();
//        String serialNumber = in.readString();
        Parcel source = Parcel.obtain();
        source.writeString(name);//name
        source.writeInt(0);//vendorId
        source.writeInt(0);//productId
        source.writeInt(0);//clasz
        source.writeInt(0);//subClass
        source.writeInt(0);//protocol
        source.writeString(".");//manufacturerName
        source.writeString(".");//productName
        source.writeString(".");//version
        source.writeString(".");//serialNumber
        source.setDataPosition(0);
        return UsbDevice.CREATOR.createFromParcel(source);
    }
}
