#include "UVCCamera.h"
#include "UVCCapture.h"
#include "V4L2Capture.h"
#include "MjpegPreview.h"

UVCCamera::UVCCamera(void)
        : mCapture(NULL),
          mPreview(NULL)
{

}

UVCCamera::~UVCCamera(void)
{

}

bool UVCCamera::isConnected()
{
    if (LIKELY(mCapture))
        return mCapture->isOpened();
    return false;
}

bool UVCCamera::connect(const char *name, int fd)
{
    bool flag = false;
    if (0 == strncmp(USB_FS, name, strlen(USB_FS)))
    {
        if (LIKELY(fd > 0))
        {
            mCapture = new UVCCapture(process_image_callback, (void *) this, name, fd);
            flag = mCapture->Open();
            if (flag)
            {
                mPreview = new MjpegPreview;
            } else
            {
                SAFE_DELETE(mCapture);
            }
        }
    } else if (0 == strncmp(VIDEO_FS, name, strlen(VIDEO_FS)))
    {
        mCapture = new V4L2Capture(process_image_callback, (void *) this, name, fd);
        flag = mCapture->Open();
        if (flag)
        {
            mPreview = new MjpegPreview;
        } else
        {
            SAFE_DELETE(mCapture);
        }
    }
    return flag;
}

void UVCCamera::release()
{
    SAFE_DELETE(mPreview);
    SAFE_DELETE(mCapture);
}

bool UVCCamera::format(int pixelformat, int fps, int width, int height)
{
    ENTER();
    bool flag = false;
    if (LIKELY(mCapture))
    {
        flag = mCapture->format(pixelformat, fps, width, height);
        if (flag)
        {
            if (LIKELY(mPreview))
            {
                mPreview->setPreviewDisplaySize(width, height);
            }
        }
    }
    RETURN(flag, bool);
}

int UVCCamera::getPixelformat()
{
    if (LIKELY(mCapture))
        return mCapture->mPixelformat;
    return PIXEL_FORMAT_MJPEG;
}

int UVCCamera::getFps()
{
    if (LIKELY(mCapture))
        return mCapture->mFps;
    return DEFAULT_FRAME_FPS;
}

int UVCCamera::getWidth()
{
    if (LIKELY(mCapture))
        return mCapture->mWidth;
    return DEFAULT_FRAME_WIDTH;
}

int UVCCamera::getHeight()
{
    if (LIKELY(mCapture))
        return mCapture->mHeight;
    return DEFAULT_FRAME_HEIGHT;
}

//====================================================================================================

void UVCCamera::startCapture()
{
    if (LIKELY(mCapture))
        mCapture->startCapture();
}

void UVCCamera::stopCapture()
{
    if (LIKELY(mCapture))
        mCapture->stopCapture();
}

bool UVCCamera::isCapture()
{
    if (LIKELY(mCapture))
        return mCapture->isRunning();
    return false;
}

void UVCCamera::addPreviewDisplay(int id, ANativeWindow *window)
{
    if (LIKELY(mPreview))
        mPreview->addPreviewDisplay(id, window);
}

void UVCCamera::removePreviewDisplay(int id)
{
    if (LIKELY(mPreview))
        mPreview->removePreviewDisplay(id);
}

void UVCCamera::clearPreviewDisplay()
{
    if (LIKELY(mPreview))
        mPreview->clearPreviewDisplay();
}

int UVCCamera::countPreviewDisplay()
{
    if (LIKELY(mPreview))
        return mPreview->countPreviewDisplay();
    return 0;
}

void UVCCamera::startPreview()
{
    if (LIKELY(mPreview))
        mPreview->startPreview();
}

void UVCCamera::stopPreview()
{
    if (LIKELY(mPreview))
        mPreview->stopPreview();
}

bool UVCCamera::isPreview()
{
    if (LIKELY(mPreview))
        return mPreview->isRunning();
    return false;
}

//====================================================================================================
void UVCCamera::setFrameCallback(JNIEnv *env, jobject frameCallback)
{
    if (LIKELY(mCapture))
        mCapture->setFrameCallback(env, frameCallback);
}

jobject UVCCamera::captureOpenShared(JNIEnv *env, int length, jobject sharedCallback)
{
    if (LIKELY(mCapture))
        return mCapture->openShared(env, length, sharedCallback);
    return NULL;
}

void UVCCamera::captureCloseShared(JNIEnv *env)
{
    if (LIKELY(mCapture))
        mCapture->closeShared(env);
}

jobject UVCCamera::previewOpenShared(JNIEnv *env, int length, jobject sharedCallback)
{
    if (LIKELY(mPreview))
        return mPreview->openShared(env, length, sharedCallback);
    return NULL;
}

void UVCCamera::previewCloseShared(JNIEnv *env)
{
    if (LIKELY(mPreview))
        mPreview->closeShared(env);
}

//====================================================================================================
//private
void UVCCamera::process_image_callback(void *frame_data, int frame_data_size, void *vptr_args)
{
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(vptr_args);
    camera->processImage(frame_data, frame_data_size);
}

void UVCCamera::processImage(void *frame_data, int frame_data_size)
{
    if (LIKELY(mPreview))
    {
        mPreview->mFramePool->copyFrame(frame_data, frame_data_size);
    }
}