#include "MjpegPreview.h"
#include "TurboJpegUtils.h"

MjpegPreview::MjpegPreview()
{}

MjpegPreview::~MjpegPreview()
{

}

//====================================================================================================

static void
copyFrame(const uint8_t *src, uint8_t *dest, const int width, int height, const int stride_src,
          const int stride_dest)
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

void copyToSurface(uint8_t *data, ANativeWindow **window, int Width, int Height)
{
    if (LIKELY(*window))
    {
        ANativeWindow_Buffer buffer;
        if (ANativeWindow_lock(*window, &buffer, NULL) == 0)
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
}

//====================================================================================================

void MjpegPreview::handleMainLooper(JNIEnv *env)
{
    TurboJpegUtils::instance()->init();
    frame_stream_t *colors = frame_stream_create(mWidth * mHeight * 4);
    frame_stream_t *yuv = frame_stream_create(mWidth * mHeight * 3 / 2);

    while (isRunning())
    {
        frame_stream_t *frame = mFramePool->waitFrame();
        if (LIKELY(frame))
        {
            if (CALL_SUCCESS == TurboJpegUtils::instance()->mjpeg2yuvx(frame, yuv))
            {
                threadLock();
                onShared(env, yuv->data, yuv->data_bytes);
                threadUnlock();
                if (CALL_SUCCESS == TurboJpegUtils::instance()->yuvx2rgbx(yuv, colors))
                {
                    threadLock();
                    int size = mDisplayWindows.size();
                    for (int i = 0; i < size; i++)
                    {
                        display_window_t *display = mDisplayWindows.get(i);
                        if (LIKELY(display))
                        {
                            copyToSurface((uint8_t *) colors->data, &display->window, mWidth, mHeight);
                        }
                    }
                    threadUnlock();
                }
            }
            mFramePool->pushCacheFrame(frame);
        }
    }

    frame_stream_recycle(colors);
    TurboJpegUtils::instance()->uninit();
}
