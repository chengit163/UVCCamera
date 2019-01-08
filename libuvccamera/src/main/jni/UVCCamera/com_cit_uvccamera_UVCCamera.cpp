#if 1
#ifndef LOG_NDEBUG
#define	LOG_NDEBUG
#endif
#undef USE_LOGALL
#else
#define USE_LOGALL
#undef LOG_NDEBUG
#undef NDEBUG
#endif

#include "libUVCCamera.h"
#include "UVCCamera.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>

/**
 * set the value into the long field
 * @param env: this param should not be null
 * @param bullet_obj: this param should not be null
 * @param field_name
 * @params val
 */
static jlong setField_long(JNIEnv *env, jobject java_obj, const char *field_name, jlong val)
{
#if LOCAL_DEBUG
    LOGV("setField_long:");
#endif
    jclass clazz = env->GetObjectClass(java_obj);
    jfieldID field = env->GetFieldID(clazz, field_name, "J");
    if (LIKELY(field))
    {
        env->SetLongField(java_obj, field, val);
    } else
    {
        LOGE("__setField_long:field '%s' not found", field_name);
    }
#ifdef ANDROID_NDK
    env->DeleteLocalRef(clazz);
#endif
    return val;
}

/**
 * @param env: this param should not be null
 * @param bullet_obj: this param should not be null
 */
static jlong
__setField_long(JNIEnv *env, jobject java_obj, jclass clazz, const char *field_name, jlong val)
{
#if LOCAL_DEBUG
    LOGV("__setField_long:");
#endif
    jfieldID field = env->GetFieldID(clazz, field_name, "J");
    if (LIKELY(field))
    {
        env->SetLongField(java_obj, field, val);
    } else
    {
        LOGE("__setField_long:field '%s' not found", field_name);
    }
    return val;
}

/**
 * @param env: this param should not be null
 * @param bullet_obj: this param should not be null
 */
jint __setField_int(JNIEnv *env, jobject java_obj, jclass clazz, const char *field_name, jint val)
{
    LOGV("__setField_int:");
    jfieldID id = env->GetFieldID(clazz, field_name, "I");
    if (LIKELY(id))
    {
        env->SetIntField(java_obj, id, val);
    } else
    {
        LOGE("__setField_int:field '%s' not found", field_name);
        env->ExceptionClear();    // clear java.lang.NoSuchFieldError exception
    }
    return val;
}

/**
 * set the value into int field
 * @param env: this param should not be null
 * @param java_obj: this param should not be null
 * @param field_name
 * @params val
 */
jint setField_int(JNIEnv *env, jobject java_obj, const char *field_name, jint val)
{
    LOGV("setField_int:");
    jclass clazz = env->GetObjectClass(java_obj);
    __setField_int(env, java_obj, clazz, field_name, val);
#ifdef ANDROID_NDK
    env->DeleteLocalRef(clazz);
#endif
    return val;
}

//====================================================================================================
//====================================================================================================
//====================================================================================================

/**
 *
 * @param env
 * @param thiz
 * @return
 */
static ID_TYPE nativeCreate(JNIEnv *env, jobject thiz)
{
    ENTER();
    UVCCamera *camera = new UVCCamera();
    setField_long(env, thiz, "mNativePtr", reinterpret_cast<ID_TYPE>(camera));
    RETURN(reinterpret_cast<ID_TYPE>(camera), ID_TYPE);
}

/**
 *
 * @param env
 * @param thiz
 * @param id_camera
 */
static void nativeDestroy(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    setField_long(env, thiz, "mNativePtr", 0);
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        SAFE_DELETE(camera);
    }
    EXIT();
}

//====================================================================================================
//====================================================================================================
//====================================================================================================

static jboolean nativeIsConnected(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    jboolean isConnected = JNI_FALSE;
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        isConnected = camera->isConnected() ? JNI_TRUE : JNI_FALSE;
    }
    RETURN(isConnected, jboolean);
}

static jboolean nativeConnect(JNIEnv *env, jobject thiz, ID_TYPE id_camera,
                              jstring name_str, jint fd)
{
    ENTER();
    jboolean isConnected = JNI_FALSE;
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera && name_str))
    {
        const char *c_name = env->GetStringUTFChars(name_str, JNI_FALSE);
        isConnected = camera->connect(c_name, fd) ? JNI_TRUE : JNI_FALSE;
        env->ReleaseStringUTFChars(name_str, c_name);
    }
    RETURN(isConnected, jboolean);
}

static void nativeRelease(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        camera->release();
    }
    PRE_EXIT();
}

static jboolean nativeFormat(JNIEnv *env, jobject thiz, ID_TYPE id_camera,
                             jint pixelformat, jint fps, jint width, jint height)
{
    ENTER();
    jboolean result = JNI_FALSE;
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        result = camera->format(pixelformat, fps, width, height) ? JNI_TRUE : JNI_FALSE;
    }
    RETURN(result, jboolean);
}

static jint nativeGetPixelformat(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    jint pixelformat = PIXEL_FORMAT_MJPEG;
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        pixelformat = camera->getPixelformat();
    }
    RETURN(pixelformat, jint);
}

static jint nativeGetFps(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    jint fps = DEFAULT_FRAME_FPS;
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        fps = camera->getFps();
    }
    RETURN(fps, jint);
}

static jint nativeGetWidth(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    jint width = DEFAULT_FRAME_WIDTH;
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        width = camera->getWidth();
    }
    RETURN(width, jint);
}

static jint nativeGetHeight(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    jint heigh = DEFAULT_FRAME_HEIGHT;
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        heigh = camera->getHeight();
    }
    RETURN(heigh, jint);
}

//====================================================================================================

static void nativeStartCapture(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        camera->startCapture();
    }
    PRE_EXIT();
}

static void nativeStopCapture(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        camera->stopCapture();
    }
    PRE_EXIT();
}

static jboolean nativeIsCapture(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    jboolean isConnected = JNI_FALSE;
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        isConnected = camera->isCapture() ? JNI_TRUE : JNI_FALSE;
    }
    RETURN(isConnected, jboolean);
}

static void nativeAddPreviewDisplay(JNIEnv *env, jobject thiz, ID_TYPE id_camera,
                                    jint id, jobject surface)
{
    ENTER();
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        ANativeWindow *display = ANativeWindow_fromSurface(env, surface);
        camera->addPreviewDisplay(id, display);
    }
    PRE_EXIT();
}

static void nativeRemovePreviewDisplay(JNIEnv *env, jobject thiz, ID_TYPE id_camera,
                                       jint id)
{
    ENTER();
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        camera->removePreviewDisplay(id);
    }
    PRE_EXIT();
}

static void nativeClearPreviewDisplay(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        camera->clearPreviewDisplay();
    }
    PRE_EXIT();
}

static jint nativeCountPreviewDisplay(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    int size = 0;
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        size = camera->countPreviewDisplay();
    }
    RETURN(size, jint);
}


static void nativeStartPreview(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        camera->startPreview();
    }
    PRE_EXIT();
}

static void nativeStopPreview(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        camera->stopPreview();
    }
    PRE_EXIT();
}

static jboolean nativeIsPreview(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    jboolean isConnected = JNI_FALSE;
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        isConnected = camera->isPreview() ? JNI_TRUE : JNI_FALSE;
    }
    RETURN(isConnected, jboolean);
}

//====================================================================================================

static void nativeSetFrameCallback(JNIEnv *env, jobject thiz, ID_TYPE id_camera,
                                   jobject frameCallback)
{
    ENTER();
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        camera->setFrameCallback(env, frameCallback);
    }
    PRE_EXIT();
}

static jobject nativeCaptureOpenShared(JNIEnv *env, jobject thiz, ID_TYPE id_camera,
                                       jint length, jobject sharedCallback)
{
    ENTER();
    jobject result = NULL;
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        result = camera->captureOpenShared(env, length, sharedCallback);
    }
    RETURN(result, jobject);
}

static void nativeCaptureCloseShared(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        camera->captureCloseShared(env);
    }
    PRE_EXIT();
}

static jobject nativePreviewOpenShared(JNIEnv *env, jobject thiz, ID_TYPE id_camera,
                                       jint length, jobject sharedCallback)
{
    ENTER();
    jobject result = NULL;
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        result = camera->previewOpenShared(env, length, sharedCallback);
    }
    RETURN(result, jobject);
}

static void nativePreviewCloseShared(JNIEnv *env, jobject thiz, ID_TYPE id_camera)
{
    ENTER();
    UVCCamera *camera = reinterpret_cast<UVCCamera *>(id_camera);
    if (LIKELY(camera))
    {
        camera->previewCloseShared(env);
    }
    PRE_EXIT();
}

//====================================================================================================
//====================================================================================================
//====================================================================================================

jint registerNativeMethods(JNIEnv *env, const char *class_name, JNINativeMethod *methods,
                           int num_methods)
{
    int result = 0;
    jclass clazz = env->FindClass(class_name);
    if (LIKELY(clazz))
    {
        int result = env->RegisterNatives(clazz, methods, num_methods);
        if (UNLIKELY(result < 0))
        {
            LOGE("registerNativeMethods failed(class=%s)", class_name);
        }
    } else
    {
        LOGE("registerNativeMethods: class'%s' not found", class_name);
    }
    return result;
}

static JNINativeMethod methods[] = {
        {"nativeCreate",               "()J",                                                            (void *) nativeCreate},
        {"nativeDestroy",              "(J)V",                                                           (void *) nativeDestroy},
        //====================================================================================================
        {"nativeIsConnected",          "(J)Z",                                                           (void *) nativeIsConnected},
        {"nativeConnect",              "(JLjava/lang/String;I)Z",                                        (void *) nativeConnect},
        {"nativeRelease",              "(J)V",                                                           (void *) nativeRelease},
        {"nativeFormat",               "(JIIII)Z",                                                       (void *) nativeFormat},
        {"nativeGetPixelformat",       "(J)I",                                                           (void *) nativeGetPixelformat},
        {"nativeGetFps",               "(J)I",                                                           (void *) nativeGetFps},
        {"nativeGetWidth",             "(J)I",                                                           (void *) nativeGetWidth},
        {"nativeGetHeight",            "(J)I",                                                           (void *) nativeGetHeight},
        //
        {"nativeStartCapture",         "(J)V",                                                           (void *) nativeStartCapture},
        {"nativeStopCapture",          "(J)V",                                                           (void *) nativeStopCapture},
        {"nativeIsCapture",            "(J)Z",                                                           (void *) nativeIsCapture},
        //
        {"nativeAddPreviewDisplay",    "(JILandroid/view/Surface;)V",                                    (void *) nativeAddPreviewDisplay},
        {"nativeRemovePreviewDisplay", "(JI)V",                                                          (void *) nativeRemovePreviewDisplay},
        {"nativeClearPreviewDisplay",  "(J)V",                                                           (void *) nativeClearPreviewDisplay},
        {"nativeCountPreviewDisplay",  "(J)I",                                                           (void *) nativeCountPreviewDisplay},
        {"nativeStartPreview",         "(J)V",                                                           (void *) nativeStartPreview},
        {"nativeStopPreview",          "(J)V",                                                           (void *) nativeStopPreview},
        {"nativeIsPreview",            "(J)Z",                                                           (void *) nativeIsPreview},
        //
        {"nativeSetFrameCallback",     "(JLcom/cit/uvccamera/IFrameCallback;)V",                         (void *) nativeSetFrameCallback},
        {"nativeCaptureOpenShared",    "(JILcom/cit/uvccamera/ISharedCallback;)Landroid/os/MemoryFile;", (void *) nativeCaptureOpenShared},
        {"nativeCaptureCloseShared",   "(J)V",                                                           (void *) nativeCaptureCloseShared},
        {"nativePreviewOpenShared",    "(JILcom/cit/uvccamera/ISharedCallback;)Landroid/os/MemoryFile;", (void *) nativePreviewOpenShared},
        {"nativePreviewCloseShared",   "(J)V",                                                           (void *) nativePreviewCloseShared},
};

int register_native_methods(JNIEnv *env)
{
    LOGV("register_native_methods:");
    if (registerNativeMethods(env, JTYPE_CLASS_NAME, methods, NUM_ARRAY_ELEMENTS(methods)) < 0)
    {
        return -1;
    }
    return 0;
}