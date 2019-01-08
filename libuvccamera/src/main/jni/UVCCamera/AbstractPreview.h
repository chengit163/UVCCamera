#ifndef ABSTRACTPREVIEW_H
#define ABSTRACTPREVIEW_H

#include "ILooper.h"
#include "objectarray.h"
#include "objcet_frame_stream.h"
#include "FramePool.h"
#include "Shared.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>

#define DEFAULT_WIDTH     1280
#define DEFAULT_HEIGHT    720


typedef struct display_window
{
    int id;
    ANativeWindow *window;
} display_window_t;

class AbstractPreview : public ILooper
{
public:
    AbstractPreview();
    virtual ~AbstractPreview();

public:
    int mWidth;
    int mHeight;
    FramePool *mFramePool;
    ObjectArray<display_window_t *> mDisplayWindows;

private:
    bool mIsDeleteing;
    //
    Shared *mShared;

public:
    void setPreviewDisplaySize(int w, int h);
    void addPreviewDisplay(int id, ANativeWindow *window);
    void removePreviewDisplay(int id);
    void clearPreviewDisplay();
    int countPreviewDisplay();
    void lostDisplayWindow(display_window_t *window);
    //
    void startPreview();
    void stopPreview();
    bool isCancelable();
    virtual void handleMainLooper(JNIEnv *env) = 0;
    void afterHandleMainLooper(JNIEnv *env);
    //
    jobject openShared(JNIEnv *env, int length, jobject sharedCallback);
    void closeShared(JNIEnv *env);
    void onShared(JNIEnv *env, void *frame_data, int frame_data_size);

private:
    display_window_t *createDisplayWindow(int id, ANativeWindow *window);
    void recycleDisplayWindow(display_window_t *window);
};


#endif //ABSTRACTPREVIEW_H
