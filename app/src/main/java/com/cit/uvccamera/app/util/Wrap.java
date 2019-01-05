package com.cit.uvccamera.app.util;

import android.util.Log;

import com.blankj.utilcode.util.LogUtils;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Proxy;
import java.util.Arrays;

public class Wrap<T>
{
    /**
     * @param target
     * @param <T>
     * @return
     * @see com.cit.uvccamera.aidl.CoreClient#listUsbDevice
     */
    public static <T> T wrap(T target)
    {
        ClassLoader loader = target.getClass().getClassLoader();
        Class<?>[] interfaces = target.getClass().getInterfaces();
        InvocationHandler h = (proxy, method, args) ->
        {
            StringBuilder sb = new StringBuilder();
            try
            {
                sb.append(Thread.currentThread().getName()).append("\n");
                sb.append(target.getClass().getName()).append("#").append(method.getName()).append("\n");
                sb.append(Arrays.toString(args)).append("\n");
                Object result = method.invoke(target, args);
                sb.append(result);
                return result;
            } catch (Throwable e)
            {
                throw e;
            } finally
            {
                LogUtils.i(sb.toString());
            }
        };
        return (T) Proxy.newProxyInstance(loader, interfaces, h);
    }
}
