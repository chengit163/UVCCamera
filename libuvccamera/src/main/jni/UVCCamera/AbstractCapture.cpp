#include "AbstractCapture.h"

AbstractCapture::AbstractCapture
        (process_image_callback_t *callback, void *ptr, const char *name, int fd)
        : mIsDeleteing(false),
          mCallback(callback),
          mPtr(ptr),
          mIsOpened(false),
          mPixelformat(PIXEL_FORMAT_MJPEG),
          mFps(DEFAULT_FRAME_FPS),
          mWidth(DEFAULT_FRAME_WIDTH),
          mHeight(DEFAULT_FRAME_HEIGHT),
          mFrameCallbackObj(NULL),
          mShared(NULL)
{
    int length = strlen(name);
    mName = (char *) malloc(length + 1);
    memset(mName, 0, length + 1);
    memcpy(mName, name, length);
    mFd = fd;
    LOGI("{name: %s, fd: %d}", mName, mFd);
}

AbstractCapture::~AbstractCapture()
{
    mIsDeleteing = true;
    stopCapture();
    SAFE_FREE(mName);
    mCallback = NULL;
    mPtr = NULL;
}

//====================================================================================================

bool AbstractCapture::isOpened()
{
    return mIsOpened;
}

void AbstractCapture::formatChange()
{

}

bool AbstractCapture::format(int pixelformat, int fps, int width, int height)
{
    if (!isRunning())
    {
        bool flag = (mPixelformat != pixelformat)
                    || (mFps != fps)
                    || (mWidth != width)
                    || (mHeight != height);
        mPixelformat = pixelformat;
        mFps = fps;
        mWidth = width;
        mHeight = height;
        if (flag)
            formatChange();
        return true;
    }
    return false;
}

//====================================================================================================

void AbstractCapture::processImageCallback(JNIEnv *env, void *frame_data, int frame_data_size)
{
    if (mCallback != NULL && mPtr != NULL)
    {
        mCallback(frame_data, frame_data_size, mPtr);
    }
    threadLock();
    onFrame(env, frame_data, frame_data_size);
    onShared(env, frame_data, frame_data_size);
    threadUnlock();
}

//====================================================================================================
void AbstractCapture::startCapture()
{
    startLooper();
}

void AbstractCapture::stopCapture()
{
    stopLooper();
}

bool AbstractCapture::isCancelable()
{
    // 停止条件
    // mIsDeleteing = true: 析构
    // 非（分享）:
    return mIsDeleteing || !(LIKELY(mShared) || LIKELY(mFrameCallbackObj));
}


void AbstractCapture::afterHandleMainLooper(JNIEnv *env)
{
    if (LIKELY(mShared))
    {
        mShared->closeShared(env);
        delete mShared;
        mShared = NULL;
    }
    if (LIKELY(mFrameCallbackObj))
    {
        env->DeleteGlobalRef(mFrameCallbackObj);
        mFrameCallbackObj = NULL;
    }
}

//====================================================================================================

void AbstractCapture::setFrameCallback(JNIEnv *env, jobject frameCallback)
{
    threadLock();
    if (LIKELY(frameCallback))
    {
        jobject frameCallbackObj = env->NewGlobalRef(frameCallback);
        if (!env->IsSameObject(mFrameCallbackObj, frameCallbackObj))
        {
            iframeCallback_fields.onFrame = NULL;
            if (mFrameCallbackObj)
            {
                env->DeleteGlobalRef(mFrameCallbackObj);
                mFrameCallbackObj = NULL;
            }
            mFrameCallbackObj = frameCallbackObj;
            if (frameCallbackObj)
            {
                jclass clazz = env->GetObjectClass(frameCallbackObj);
                if (LIKELY(clazz))
                {
                    iframeCallback_fields.onFrame =
                            env->GetMethodID(clazz, "onFrame", "(Ljava/nio/ByteBuffer;)V");
                } else
                {
                    LOGW("failed to get object class");
                }
                env->ExceptionClear();
                if (!iframeCallback_fields.onFrame)
                {
                    LOGE("Can't find IFrameCallback#onFrame");
                    env->DeleteGlobalRef(frameCallbackObj);
                    mFrameCallbackObj = frameCallbackObj = NULL;
                }
            }
        }
    } else
    {
        if (LIKELY(mFrameCallbackObj))
        {
            env->DeleteGlobalRef(mFrameCallbackObj);
            mFrameCallbackObj = NULL;
        }
    }
    threadUnlock();
}

jobject AbstractCapture::openShared(JNIEnv *env, int length, jobject sharedCallback)
{
    jobject obj = NULL;
    threadLock();
    if (!LIKELY(mShared))
        mShared = new Shared;
    obj = mShared->openShared(env, length, sharedCallback);
    threadUnlock();
    return obj;
}

void AbstractCapture::closeShared(JNIEnv *env)
{
    threadLock();
    if (LIKELY(mShared))
    {
        mShared->closeShared(env);
        delete mShared;
        mShared = NULL;
    }
    threadUnlock();
}

void AbstractCapture::onShared(JNIEnv *env, void *frame_data, int frame_data_size)
{
    if (LIKELY(mShared))
    {
        mShared->onShared(env, frame_data, frame_data_size);
    }
}

//====================================================================================================
//private
void AbstractCapture::onFrame(JNIEnv *env, void *frame_data, int frame_data_size)
{
    if (LIKELY(mFrameCallbackObj))
    {
        jobject buf = env->NewDirectByteBuffer(frame_data, frame_data_size);
        env->CallVoidMethod(mFrameCallbackObj, iframeCallback_fields.onFrame, buf);
        env->ExceptionClear();
        env->DeleteLocalRef(buf);
    }
}