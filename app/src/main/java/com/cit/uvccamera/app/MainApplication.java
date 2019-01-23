package com.cit.uvccamera.app;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Application;
import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;

import com.blankj.utilcode.util.AppUtils;
import com.blankj.utilcode.util.CrashUtils;
import com.blankj.utilcode.util.LogUtils;
import com.cit.uvccamera.app.service.ProxyService;
import com.squareup.leakcanary.LeakCanary;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

public class MainApplication extends Application
{

    private static final String TAG = "Application";

    private static MainApplication sInstance;

    public static MainApplication getInstance()
    {
        return sInstance;
    }

    @Override
    public void onCreate()
    {
        super.onCreate();
        sInstance = this;
        registerActivityLifecycleCallbacks(mCallbacks);
        com.blankj.utilcode.util.Utils.init(sInstance);
        initLeakCanary();
        initLog();
        initCrash();
        //
//        Intent service = new Intent(ProxyService.class.getName());
//        startService(service);
    }


    private void initLeakCanary()
    {
        // 内存泄露检查工具
        if (LeakCanary.isInAnalyzerProcess(this))
        {
            // This process is dedicated to LeakCanary for heap analysis.
            // You should not init your app in this process.
            return;
        }
        LeakCanary.install(this);
    }

    // init it in ur application
    public void initLog()
    {
        int pid = android.os.Process.myPid(); //进程id
        String pname = getProcessName(pid);//进程名称
        String fix = (pname == null ? "log" : Integer.toHexString(pname.hashCode()));
        StringBuilder sb = new StringBuilder();
        sb.append("\n");
        sb.append("(").append(pid).append(")");
        sb.append(pname);
        sb.append("[").append(fix).append("]");
        sb.append("\n");
        //
        final LogUtils.Config config = LogUtils.getConfig()
                .setLogSwitch(BuildConfig.DEBUG)// 设置 log 总开关，包括输出到控制台和文件，默认开
                .setConsoleSwitch(BuildConfig.DEBUG)// 设置是否输出到控制台开关，默认开
                .setGlobalTag("app")// 设置 log 全局标签，默认为空
                // 当全局标签不为空时，我们输出的 log 全部为该 tag，
                // 为空时，如果传入的 tag 为空那就显示类名，否则显示 tag
                .setLogHeadSwitch(false)// 设置 log 头信息开关，默认为开
                .setLog2FileSwitch(false)// 打印 log 时是否存到文件的开关，默认关
                .setDir("")// 当自定义路径为空时，写入应用的/cache/log/目录中
                .setFilePrefix(fix)// 当文件前缀为空时，默认为"util"，即写入文件为"util-yyyy-MM-dd.txt"
                .setBorderSwitch(true)// 输出日志是否带边框开关，默认开
                .setSingleTagSwitch(true)// 一条日志仅输出一条，默认开，为美化 AS 3.1 的 Logcat
                .setConsoleFilter(LogUtils.V)// log 的控制台过滤器，和 logcat 过滤器同理，默认 Verbose
                .setFileFilter(LogUtils.V)// log 文件过滤器，和 logcat 过滤器同理，默认 Verbose
                .setStackDeep(1)// log 栈深度，默认为 1
                .setStackOffset(0)// 设置栈偏移，比如二次封装的话就需要设置，默认为 0
                .setSaveDays(3)// 设置日志可保留天数，默认为 -1 表示无限时长
                // 新增 ArrayList 格式化器，默认已支持 Array, Throwable, Bundle, Intent 的格式化输出
                .addFormatter(new LogUtils.IFormatter<ArrayList>()
                {
                    @Override
                    public String format(ArrayList list)
                    {
                        return "LogUtils Formatter ArrayList { " + list.toString() + " }";
                    }
                });
        LogUtils.i(sb.toString());
        LogUtils.d(config.toString());
    }

    @SuppressLint("MissingPermission")
    private void initCrash()
    {
        CrashUtils.init(new CrashUtils.OnCrashListener()
        {
            @Override
            public void onCrash(String crashInfo, Throwable e)
            {
                LogUtils.e(crashInfo);
                AppUtils.relaunchApp();
            }
        });
    }

    /**
     * 获取进程号对应的进程名
     *
     * @param pid 进程号
     * @return 进程名
     */
    public static String getProcessName(int pid)
    {
        BufferedReader reader = null;
        try
        {
            reader = new BufferedReader(new FileReader("/proc/" + pid + "/cmdline"));
            String processName = reader.readLine();
            if (!TextUtils.isEmpty(processName))
            {
                processName = processName.trim();
            }
            return processName;
        } catch (Throwable throwable)
        {
            throwable.printStackTrace();
        } finally
        {
            try
            {
                if (reader != null)
                {
                    reader.close();
                }
            } catch (IOException exception)
            {
                exception.printStackTrace();
            }
        }
        return null;
    }

    private ActivityLifecycleCallbacks mCallbacks = new ActivityLifecycleCallbacks()
    {

        @Override
        public void onActivityCreated(Activity activity, Bundle savedInstanceState)
        {
            Log.d(TAG, "onActivityCreated() called with: activity = [" + activity + "], savedInstanceState = [" + savedInstanceState + "]");
        }

        @Override
        public void onActivityStarted(Activity activity)
        {
            Log.d(TAG, "onActivityStarted() called with: activity = [" + activity + "]");
        }

        @Override
        public void onActivityResumed(Activity activity)
        {
            Log.d(TAG, "onActivityResumed() called with: activity = [" + activity + "]");
        }

        @Override
        public void onActivityPaused(Activity activity)
        {
            Log.d(TAG, "onActivityPaused() called with: activity = [" + activity + "]");
        }

        @Override
        public void onActivityStopped(Activity activity)
        {
            Log.d(TAG, "onActivityStopped() called with: activity = [" + activity + "]");
        }

        @Override
        public void onActivitySaveInstanceState(Activity activity, Bundle outState)
        {
            Log.d(TAG, "onActivitySaveInstanceState() called with: activity = [" + activity + "], outState = [" + outState + "]");
        }

        @Override
        public void onActivityDestroyed(Activity activity)
        {
            Log.d(TAG, "onActivityDestroyed() called with: activity = [" + activity + "]");
        }
    };
}
