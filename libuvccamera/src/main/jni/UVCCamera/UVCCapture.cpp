#include "UVCCapture.h"

UVCCapture::UVCCapture
        (process_image_callback_t *callback, void *ptr, const char *name, int fd)
        : AbstractCapture(callback, ptr, name, fd),
          mContext(NULL),
          mDevice(NULL),
          mDeviceHandle(NULL)
{
    mFd = dup(mFd);
    mFramePool = new FramePool(this);
}

UVCCapture::~UVCCapture()
{
    stopCapture();
    Close();
    SAFE_DELETE(mFramePool);
}

bool UVCCapture::Open(void)
{
    uvc_error_t result = uvc_init2(&mContext, NULL, USB_FS);
    if (LIKELY(!result))
    {
        result = uvc_find_device3(mContext, &mDevice, mName, mFd);
        if (LIKELY(!result))
        {
            result = uvc_open(mDevice, &mDeviceHandle);
            if (LIKELY(!result))
            {
                mIsOpened = true;
            }
        }
    }
    return mIsOpened;
}

void UVCCapture::Close(void)
{
    if (NULL != mDeviceHandle)
    {
        uvc_close(mDeviceHandle);
        mDeviceHandle = NULL;
    }
    if (NULL != mDevice)
    {
        uvc_unref_device(mDevice);
        mDevice = NULL;
    }
    if (NULL != mContext)
    {
        uvc_exit(mContext);
        mContext = NULL;
    }
    if (LIKELY(mFd))
    {
        close(mFd);
        mFd = 0;
    }
    mIsOpened = false;
}

void UVCCapture::formatChange()
{
    LOGI("{pixelformat: %d, fps: %d, width: %d, height: %d}", mPixelformat, mFps, mWidth, mHeight);
}

void UVCCapture::startCapture()
{
    if (!isRunning())
    {
        if (LIKELY(mDeviceHandle))
        {
            uvc_stream_ctrl_t ctrl;
            uvc_error_t result = uvc_get_stream_ctrl_format_size(mDeviceHandle, &ctrl,
                                                                 UVC_FRAME_FORMAT_MJPEG,
                                                                 mWidth, mHeight, mFps);
            uvc_start_streaming(mDeviceHandle, &ctrl, uvc_preview_frame_callback, (void *) this, 0);
        }
        AbstractCapture::startCapture();
    }
}

void UVCCapture::stopCapture()
{
    if (isCancelable())
    {
        if (isRunning())
        {
            AbstractCapture::stopCapture();
            if (LIKELY(mDeviceHandle))
            {
                uvc_stop_streaming(mDeviceHandle);
            }
        }
    }
}

void UVCCapture::handleMainLooper(JNIEnv *env)
{
    while (isRunning())
    {
        frame_stream_t *frame = mFramePool->waitFrame();
        if (LIKELY(frame))
        {
            processImageCallback(env, frame->data, frame->data_bytes);
            mFramePool->pushCacheFrame(frame);
        }
    }
    mFramePool->clearAllFrame();
}

//====================================================================================================
//private
void UVCCapture::uvc_preview_frame_callback(uvc_frame_t *frame, void *vptr_args)
{
    if (frame && (frame->actual_bytes > 0))
    {
        UVCCapture *capture = reinterpret_cast<UVCCapture *>(vptr_args);
        capture->mFramePool->copyFrame(frame->data, frame->actual_bytes);
    }
}