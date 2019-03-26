#include "FramePool.h"

FramePool::FramePool(ILooper *looper)
        : mLooper(looper)
{
    pthread_mutex_init(&cache_mutex, NULL);
}

FramePool::~FramePool()
{
    clearAllFrame();
    pthread_mutex_destroy(&cache_mutex);
}

frame_stream_t *FramePool::popCacheFrame(int size)
{
    frame_stream_t *frame = NULL;
    pthread_mutex_lock(&cache_mutex);
    if (!mCacheFrames.isEmpty())
    {
        frame = mCacheFrames.last();
        // 判断是否够用
        if (frame->data_max < size)
        {
            frame_stream_recycle(frame);
            frame = NULL;
        }
    }
    pthread_mutex_unlock(&cache_mutex);
    if (NULL == frame)
        frame = frame_stream_create(size);
    return frame;
}

void FramePool::pushCacheFrame(frame_stream_t *frame)
{
    pthread_mutex_lock(&cache_mutex);
    int size = mCacheFrames.size();
    if (size < MAX_CACHE_SZ)
    {
        mCacheFrames.put(frame);
        frame = NULL;
    }
    pthread_mutex_unlock(&cache_mutex);
    if (NULL != frame)
        frame_stream_recycle(frame);
}

void FramePool::copyFrame(void *pFrame, int nFrame)
{
    if (mLooper->isRunning())
    {
        frame_stream_t *frame = popCacheFrame(nFrame);
        frame->data_bytes = nFrame;
        memcpy(frame->data, pFrame, nFrame);
        addFrame(frame);
    }
}


void FramePool::addFrame(frame_stream_t *frame)
{
    mLooper->threadLock();
    int size = mFrames.size();
    if (size > MAX_FRAME_SZ)
    {
        LOGW("throw frame");
        frame_stream_t *old = mFrames.remove(0);
        frame_stream_recycle(old);
    }
    mFrames.put(frame);
    mLooper->threadSignal();
    mLooper->threadUnlock();
}

frame_stream_t *FramePool::waitFrame()
{
    frame_stream_t *frame = NULL;
    mLooper->threadLock();
    if (mFrames.isEmpty())
        mLooper->threadWait();
    if (!mFrames.isEmpty())
        frame = mFrames.remove(0);
    mLooper->threadUnlock();
    return frame;
}

frame_stream_t *FramePool::waitFrameTimed(long msec)
{
    frame_stream_t *frame = NULL;
    mLooper->threadLock();
    if (mFrames.isEmpty())
        mLooper->threadTimedwait(msec);
    if (!mFrames.isEmpty())
        frame = mFrames.remove(0);
    mLooper->threadUnlock();
    return frame;
}

void FramePool::clearAllFrame()
{
    // clear mPreviewFrames 清空预览流
    mLooper->threadLock();
    while (!mFrames.isEmpty())
    {
        frame_stream_recycle(mFrames.last());
    }
    mLooper->threadUnlock();
    // clear mPoolFrames 清空缓存流
    pthread_mutex_lock(&cache_mutex);
    while (!mCacheFrames.isEmpty())
    {
        frame_stream_recycle(mCacheFrames.last());
    }
    pthread_mutex_unlock(&cache_mutex);
}
