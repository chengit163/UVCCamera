#ifndef ILOOPER_H
#define ILOOPER_H

#include <pthread.h>
#include "_utilbase.h"

class ILooper
{
public:
    ILooper(void);

    virtual ~ILooper(void);

private:
    bool mIsRunning;
    pthread_t looper_thread;
    pthread_mutex_t looper_mutex;
    pthread_cond_t looper_sync;

public:
    void threadLock();
    void threadUnlock();
    void threadSignal();
    void threadWait();
    void threadTimedwait(long msec);
    bool isRunning();
    void startLooper();
    void stopLooper();
    virtual void handleMainLooper(JNIEnv *env) = 0;
    virtual void afterHandleMainLooper(JNIEnv *env) = 0;

private:
    static void *looper_thread_func(void *vptr_args);
    void looperThreadFunc();
};

#endif //ILOOPER_H
