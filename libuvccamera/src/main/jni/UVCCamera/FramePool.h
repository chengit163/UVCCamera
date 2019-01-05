#ifndef FRAMEPOOL_H
#define FRAMEPOOL_H

#include "ILooper.h"
#include "objectarray.h"
#include "objcet_frame_stream.h"

class FramePool
{
public:
    FramePool(ILooper *looper);
    virtual ~FramePool();

private:
    ILooper *mLooper;
    pthread_mutex_t cache_mutex;
    ObjectArray<frame_stream_t *> mFrames;
    ObjectArray<frame_stream_t *> mCacheFrames;

public:
    frame_stream_t *popCacheFrame(int size);
    void pushCacheFrame(frame_stream_t *frame);
    void copyFrame(void *pFrame, int nFrame);
    void addFrame(frame_stream_t *frame);
    frame_stream_t *waitFrame();
    frame_stream_t *waitFrameTimed(long msec);
    void clearAllFrame();
};


#endif //FRAMEPOOL_H
