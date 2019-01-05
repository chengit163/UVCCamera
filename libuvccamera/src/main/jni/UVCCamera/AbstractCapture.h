#ifndef ABSTRACTCAPTURE_H
#define ABSTRACTCAPTURE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ILooper.h"
#include "Shared.h"

#define PIXEL_FORMAT_MJPEG      0
#define PIXEL_FORMAT_H264       1

#define DEFAULT_FRAME_FPS       30
#define DEFAULT_FRAME_WIDTH     1280
#define DEFAULT_FRAME_HEIGHT    720

#define USB_FS          "/dev/bus/usb"
#define VIDEO_FS        "/dev/video"


typedef void(process_image_callback_t)(void *frame_data, int frame_data_size, void *vptr_args);

typedef struct
{
    jmethodID onFrame;
} Fields_iframeCallback;

class AbstractCapture : public ILooper
{
public:
    AbstractCapture(process_image_callback_t *callback, void *ptr, const char *name, int fd);
    virtual ~AbstractCapture();

public:
    char *mName;
    int mFd;
    bool mIsOpened;
    int mPixelformat;
    int mFps;
    int mWidth;
    int mHeight;

private:
    bool mIsDeleteing;
    //
    process_image_callback_t *mCallback;
    void *mPtr;
    //
    jobject mFrameCallbackObj;
    Fields_iframeCallback iframeCallback_fields;
    Shared *mShared;

public:
    bool isOpened();
    virtual bool Open(void) = 0;
    virtual void Close(void) = 0;
    virtual void formatChange();
    bool format(int pixelformat, int fps, int width, int height);
    //
    void processImageCallback(JNIEnv *env, void *frame_data, int frame_data_size);
    //
    virtual void startCapture();
    virtual void stopCapture();
    bool isCancelable();
    virtual void handleMainLooper(JNIEnv *env) = 0;
    void afterHandleMainLooper(JNIEnv *env);
    //
    void setFrameCallback(JNIEnv *env, jobject frameCallback);
    jobject openShared(JNIEnv *env, int length, jobject sharedCallback);
    void closeShared(JNIEnv *env);
    void onShared(JNIEnv *env, void *frame_data, int frame_data_size);

private:
    void onFrame(JNIEnv *env, void *frame_data, int frame_data_size);
};

#endif //ABSTRACTCAPTURE_H
