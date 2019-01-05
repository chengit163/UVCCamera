#ifndef UVCCAPTURE_H
#define UVCCAPTURE_H

#include "AbstractCapture.h"
#include "FramePool.h"

#if 1
#include "../libusb/libusb/libusb.h"
#include "../libuvc/include/libuvc/libuvc.h"
#else
#include "libusb.h"
#include "libuvc.h"
#endif

class UVCCapture : public AbstractCapture
{
public:
    UVCCapture(process_image_callback_t *callback, void *ptr, const char *name, int fd);
    virtual ~UVCCapture();

private:
    uvc_context_t *mContext;
    uvc_device_t *mDevice;
    uvc_device_handle_t *mDeviceHandle;
    FramePool *mFramePool;

public:
    virtual bool Open(void);
    virtual void Close(void);
    virtual void formatChange();
    virtual void startCapture();
    virtual void stopCapture();
    virtual void handleMainLooper(JNIEnv *env);

private:
    static void uvc_preview_frame_callback(uvc_frame_t *frame, void *vptr_args);
};


#endif //_UVCCAPTURE_H
