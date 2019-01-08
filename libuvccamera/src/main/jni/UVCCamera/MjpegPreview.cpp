#include "MjpegPreview.h"
#include "TurboJpegUtils.h"

MjpegPreview::MjpegPreview()
{}

MjpegPreview::~MjpegPreview()
{

}

//====================================================================================================

static void copyFrame(const uint8_t *src, uint8_t *dest, const int width, int height,
                      const int stride_src, const int stride_dest)
{
    const int h8 = height % 8;
    for (int i = 0; i < h8; i++)
    {
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
    }
    for (int i = 0; i < height; i += 8)
    {
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest;
        src += stride_src;
    }
}

int copyToSurface(uint8_t *data, ANativeWindow **window, int Width, int Height)
{
    int result = -1;
    if (LIKELY(*window))
    {
        ANativeWindow_Buffer buffer;
        if ((result = ANativeWindow_lock(*window, &buffer, NULL)) == 0)
        {
            // source = frame data
            const uint8_t *src = data;
            const int src_w = Width * 4;
            const int src_step = Width * 4;
            // destination = Surface(ANativeWindow)
            uint8_t *dest = (uint8_t *) buffer.bits;
            const int dest_w = buffer.width * 4;
            const int dest_step = buffer.stride * 4;
            // use lower transfer bytes
            const int w = src_w < dest_w ? src_w : dest_w;
            // use lower height
            const int h = Height < buffer.height ? Height : buffer.height;
            // transfer from frame data to the Surface
            copyFrame(src, dest, w, h, src_step, dest_step);
            ANativeWindow_unlockAndPost(*window);
        }
    }
    return result;
}

//====================================================================================================

void MjpegPreview::handleMainLooper(JNIEnv *env)
{
    TurboJpegUtils::instance()->init();
    frame_stream_t *colors = frame_stream_create(mWidth * mHeight * 4);
    frame_stream_t *yuv = frame_stream_create(mWidth * mHeight * 3 / 2);
    ObjectArray<int> lostIndexs;

    while (isRunning())
    {
        frame_stream_t *frame = mFramePool->waitFrame();
        if (LIKELY(frame))
        {
            if (CALL_SUCCESS == TurboJpegUtils::instance()->mjpeg2yuvx(frame, yuv))
            {
                int size = 0; // 预览数
                threadLock();
                onShared(env, yuv->data, yuv->data_bytes);
                size = mDisplayWindows.size();// 初次获取预览数
                threadUnlock();
                if (size > 0)
                {
                    // 预览数大于0，解码yuv
                    if (CALL_SUCCESS == TurboJpegUtils::instance()->yuvx2rgbx(yuv, colors))
                    {
                        threadLock();
                        size = mDisplayWindows.size();//再次获取预览数
                        for (int i = 0; i < size; i++)
                        {
                            display_window_t *display = mDisplayWindows.get(i);
                            if (LIKELY(display))
                            {
                                int result = copyToSurface((uint8_t *) colors->data,
                                                           &display->window,
                                                           mWidth, mHeight);
                                if (0 != result)
                                {
                                    int index = i + 1;//避免 if LIKELY(0) (+1非零)
                                    lostIndexs.put(index);//上屏失败，认为该Surface已失效
                                }
                            }
                        }
                        while (!lostIndexs.isEmpty())
                        {
                            int index = lostIndexs.last() - 1;//避免 if LIKELY(0) (-1还远)
                            display_window_t *display = mDisplayWindows.remove(index);
                            lostDisplayWindow(display);
                        }
                        threadUnlock();
                    }
                }
            }
            mFramePool->pushCacheFrame(frame);
        }
    }

    lostIndexs.clear();
    frame_stream_recycle(colors);
    TurboJpegUtils::instance()->uninit();
}
