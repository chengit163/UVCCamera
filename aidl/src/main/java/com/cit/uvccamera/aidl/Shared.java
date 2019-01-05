package com.cit.uvccamera.aidl;

import android.os.MemoryFile;
import android.os.ParcelFileDescriptor;

import java.io.FileDescriptor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

public class Shared
{

    private static final int HEADER_LENGTH = 16;
    private MemoryFile memoryFile;
    protected byte[] header;
    protected byte[] body;
    //
    private int mAvailable;

    public Shared(ParcelFileDescriptor pfd, int length)
    {
        memoryFile = createMemoryFile(pfd, length);
        header = new byte[HEADER_LENGTH];
        body = new byte[length - HEADER_LENGTH];
    }

    public void recycle()
    {
        if (memoryFile != null)
        {
            memoryFile.close();
            memoryFile = null;
        }
        header = null;
        body = null;
    }


    public int read(int available)
    {
        int n = 0;
        if (memoryFile != null)
        {
            try
            {
                memoryFile.readBytes(header, 0, 0, header.length);
                if (parse(available))
                {
                    n = memoryFile.readBytes(body, header.length, 0, mAvailable);
                }
            } catch (Exception e)
            {
                e.printStackTrace();
            }
        }
        return n;
    }


    /**
     * 解析header
     *
     * @param available
     * @return
     */
    private boolean parse(int available)
    {
        mAvailable = (header[0] & 0xff) | ((header[1] & 0xff) << 8) | ((header[2] & 0xff) << 16) | ((header[3] & 0xff) << 24);
        return mAvailable == available;
    }


    private MemoryFile createMemoryFile(ParcelFileDescriptor pfd, int length)
    {
        try
        {
            FileDescriptor fd = pfd.getFileDescriptor();
            //
            MemoryFile memoryFile = new MemoryFile(null, 0);
            memoryFile.close();
            //
            Class<?> clazz = MemoryFile.class;
            Field field = null;
            Method method = null;
            // mFD
            field = clazz.getDeclaredField("mFD");
            field.setAccessible(true);
            field.set(memoryFile, fd);
            //
            field = clazz.getDeclaredField("mLength");
            field.setAccessible(true);
            field.set(memoryFile, length);
            //
            method = clazz.getDeclaredMethod("native_mmap", FileDescriptor.class, int.class, int.class);
            method.setAccessible(true);
            Object obj = method.invoke(memoryFile, fd, length, 0x1);
            //
            field = clazz.getDeclaredField("mAddress");
            field.setAccessible(true);
            field.set(memoryFile, obj);
            return memoryFile;
        } catch (Exception e)
        {
            e.printStackTrace();
        }
        return null;
    }
}
