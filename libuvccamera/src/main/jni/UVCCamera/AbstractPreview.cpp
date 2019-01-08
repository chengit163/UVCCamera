#include "AbstractPreview.h"

AbstractPreview::AbstractPreview()
        : mIsDeleteing(false),
          mWidth(DEFAULT_WIDTH),
          mHeight(DEFAULT_HEIGHT),
          mShared(NULL)
{
    mFramePool = new FramePool(this);
}

AbstractPreview::~AbstractPreview()
{
    mIsDeleteing = true;
    stopPreview();
    clearPreviewDisplay();
    SAFE_DELETE(mFramePool);
}

//====================================================================================================

void AbstractPreview::setPreviewDisplaySize(int w, int h)
{
    mWidth = w;
    mHeight = h;
}

void AbstractPreview::addPreviewDisplay(int id, ANativeWindow *window)
{
    threadLock();
    display_window_t *display_window = createDisplayWindow(id, window);
    mDisplayWindows.put(display_window);
    threadUnlock();
}

void AbstractPreview::removePreviewDisplay(int id)
{
    threadLock();
    int index = -1;
    int size = mDisplayWindows.size();
    for (int i = 0; i < size; i++)
    {
        display_window_t *display = mDisplayWindows.get(i);
        if (NULL != display && id == display->id)
        {
            index = i;
            break;
        }
    }
    if (-1 != index)
    {
        display_window_t *display = mDisplayWindows.remove(index);
        recycleDisplayWindow(display);
    }
    threadUnlock();
}

void AbstractPreview::clearPreviewDisplay()
{
    threadLock();
    while (!mDisplayWindows.isEmpty())
    {
        recycleDisplayWindow(mDisplayWindows.last());
    }
    threadUnlock();
}

int AbstractPreview::countPreviewDisplay()
{
    return mDisplayWindows.size();
}

void AbstractPreview::lostDisplayWindow(display_window_t *window)
{
    if (LIKELY(window))
    {
        LOGW("lost surface: %d", window->id);
        recycleDisplayWindow(window);
    }
}

//====================================================================================================
void AbstractPreview::startPreview()
{
    startLooper();
}

void AbstractPreview::stopPreview()
{
    if (isCancelable())
        stopLooper();
}

bool AbstractPreview::isCancelable()
{
    // 停止条件
    // mIsDeleteing = true: 析构
    // 非（分享）:
    return mIsDeleteing || !(LIKELY(mShared));
}

void AbstractPreview::afterHandleMainLooper(JNIEnv *env)
{
    if (LIKELY(mShared))
    {
        mShared->closeShared(env);
        delete mShared;
        mShared = NULL;
    }
}
//====================================================================================================

jobject AbstractPreview::openShared(JNIEnv *env, int length, jobject sharedCallback)
{
    jobject obj = NULL;
    threadLock();
    if (!LIKELY(mShared))
        mShared = new Shared;
    obj = mShared->openShared(env, length, sharedCallback);
    threadUnlock();
    return obj;
}

void AbstractPreview::closeShared(JNIEnv *env)
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

void AbstractPreview::onShared(JNIEnv *env, void *frame_data, int frame_data_size)
{
    if (LIKELY(mShared))
    {
        mShared->onShared(env, frame_data, frame_data_size);
    }
}

//====================================================================================================
//private
display_window_t *AbstractPreview::createDisplayWindow(int id, ANativeWindow *window)
{
    ANativeWindow_setBuffersGeometry(window, mWidth, mHeight, WINDOW_FORMAT_RGBA_8888);
    display_window_t *display_window = new display_window_t;
    display_window->id = id;
    display_window->window = window;
    return display_window;
}

void AbstractPreview::recycleDisplayWindow(display_window_t *display_window)
{
    if (NULL != display_window)
    {
        if (NULL != display_window->window)
        {
            ANativeWindow_release(display_window->window);
            display_window->window = NULL;
        }
        delete display_window;
        display_window = NULL;
    }
}







