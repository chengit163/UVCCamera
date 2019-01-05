#ifndef UVCCAMERA_H
#define UVCCAMERA_H

#include "AbstractCapture.h"
#include "AbstractPreview.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>

class UVCCamera
{
public:
    UVCCamera(void);

    virtual ~UVCCamera(void);

public:
    AbstractCapture *mCapture;
    AbstractPreview *mPreview;

public:
    //
    bool isConnected();
    bool connect(const char *filename, int fd);
    void release();
    bool format(int pixelformat, int fps, int width, int height);
    int getPixelformat();
    int getFps();
    int getWidth();
    int getHeight();
    //
    void startCapture();
    void stopCapture();
    bool isCapture();
    //
    void addPreviewDisplay(int id, ANativeWindow *window);
    void removePreviewDisplay(int id);
    void clearPreviewDisplay();
    int countPreviewDisplay();
    void startPreview();
    void stopPreview();
    bool isPreview();
    //
    void setFrameCallback(JNIEnv *env, jobject frameCallback);
    jobject captureOpenShared(JNIEnv *env, int length, jobject sharedCallback);
    void captureCloseShared(JNIEnv *env);
    jobject previewOpenShared(JNIEnv *env, int length, jobject sharedCallback);
    void previewCloseShared(JNIEnv *env);
private:
    static void process_image_callback(void *frame_data, int frame_data_size, void *vptr_args);
    void processImage(void *frame_data, int frame_data_size);
};

#endif //UVCCAMERA_H
