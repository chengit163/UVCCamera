#include "ILooper.h"

ILooper::ILooper(void)
        : mIsRunning(false),
          looper_thread(-1)
{
    pthread_cond_init(&looper_sync, NULL);
    pthread_mutex_init(&looper_mutex, NULL);
}

ILooper::~ILooper(void)
{
    pthread_mutex_destroy(&looper_mutex);
    pthread_cond_destroy(&looper_sync);
}

void ILooper::threadLock()
{
    pthread_mutex_lock(&looper_mutex);
}

void ILooper::threadUnlock()
{
    pthread_mutex_unlock(&looper_mutex);
}

void ILooper::threadSignal()
{
    pthread_cond_signal(&looper_sync);
}

void ILooper::threadWait()
{
    pthread_cond_wait(&looper_sync, &looper_mutex);
}

void ILooper::threadTimedwait(long msec)
{
    struct timespec outtime;
    struct timeval now;
    gettimeofday(&now, NULL);

    long nsec = now.tv_usec * 1000 + (msec % 1000) * 1000000;
    outtime.tv_sec = now.tv_sec + nsec / 1000000000 + nsec / 1000;
    outtime.tv_nsec = nsec % 1000000000;

    pthread_cond_timedwait(&looper_sync, &looper_mutex, &outtime);
}

bool ILooper::isRunning()
{
    return mIsRunning;
}

void ILooper::startLooper()
{
    if (!isRunning())
    {
        mIsRunning = true;
        threadLock();
        {
            pthread_create(&looper_thread, NULL, looper_thread_func, (void *) this);
        }
        threadUnlock();
    }
}

void ILooper::stopLooper()
{
    if (isRunning())
    {
        mIsRunning = false;
        threadSignal();
        if (-1 != looper_thread)
        {
            pthread_join(looper_thread, NULL);
            looper_thread = -1;
        }
    }
}

void *ILooper::looper_thread_func(void *vptr_args)
{
    ILooper *obj = reinterpret_cast<ILooper *>(vptr_args);
    if (LIKELY(obj))
    {
        obj->looperThreadFunc();
    }
    return (void *) 0;
}

void ILooper::looperThreadFunc()
{
    JavaVM *vm = getVM();
    JNIEnv *env;
    // attach to JavaVM
    vm->AttachCurrentThread(&env, NULL);
    handleMainLooper(env);
    afterHandleMainLooper(env);
    // detach from JavaVM
    vm->DetachCurrentThread();
    MARK("DetachCurrentThread");
}
