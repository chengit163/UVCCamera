package com.cit.uvccamera.app.util;

import com.blankj.utilcode.util.LogUtils;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Proxy;
import java.util.Arrays;

public class Wrap<T>
{

    /**
     * 包装并返回动态代理对象<br>
     * 打印接口执行详情
     *
     * @param target
     * @param <T>
     * @return
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
                sb.append(Thread.currentThread().getName()).append("\n");//当前线程
                sb.append(target.getClass().getName()).append("#").append(method.getName()).append("\n");//类#函数
                sb.append(Arrays.toString(args)).append("\n");//参数
                Object result = method.invoke(target, args);//结果
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
