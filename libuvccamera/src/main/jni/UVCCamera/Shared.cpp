#include "Shared.h"

Shared::Shared() :
        mMemoryFileArr(NULL),
        mMemoryFileObj(NULL),
        mSharedCallbackObj(NULL)
{
}

Shared::~Shared()
{
}

jobject Shared::openShared(JNIEnv *env, int length, jobject sharedCallback)
{
    if (length > 0)
    {
        if (!LIKELY(mMemoryFileArr))
        {
            jbyteArray arr = env->NewByteArray(length);
            mMemoryFileArr = (jbyteArray) env->NewGlobalRef(arr);
        }
        if (!LIKELY(mMemoryFileObj))
        {
            memoryFile_fields.close = NULL;
            memoryFile_fields.readBytes = NULL;
            memoryFile_fields.writeBytes = NULL;
            //
            jclass clazz = env->FindClass("android/os/MemoryFile");
            jmethodID construct = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;I)V");
            jobject obj = env->NewObject(clazz, construct, NULL, length);
            mMemoryFileObj = env->NewGlobalRef(obj);
            //
            if (mMemoryFileObj)
            {
                jclass clazz = env->GetObjectClass(mMemoryFileObj);
                if (LIKELY(clazz))
                {
                    memoryFile_fields.close =
                            env->GetMethodID(clazz, "close", "()V");
                    memoryFile_fields.readBytes =
                            env->GetMethodID(clazz, "readBytes", "([BIII)I");
                    memoryFile_fields.writeBytes =
                            env->GetMethodID(clazz, "writeBytes", "([BIII)V");
                } else
                {
                    LOGW("failed to get object class");
                }
                env->ExceptionClear();
                if (!memoryFile_fields.close
                    || !memoryFile_fields.readBytes
                    || !memoryFile_fields.writeBytes)
                {
                    LOGE("Can't find MemoryFile###");
                    env->DeleteGlobalRef(mMemoryFileObj);
                    mMemoryFileObj = NULL;
                }
            }
        }
    }
    if (LIKELY(sharedCallback))
    {
        jobject sharedCallbackObj = env->NewGlobalRef(sharedCallback);
        if (!env->IsSameObject(mSharedCallbackObj, sharedCallbackObj))
        {
            isharedCallback_fields.onShared = NULL;
            if (mSharedCallbackObj)
            {
                env->DeleteGlobalRef(mSharedCallbackObj);
                mSharedCallbackObj = NULL;
            }
            mSharedCallbackObj = sharedCallbackObj;
            if (sharedCallbackObj)
            {
                jclass clazz = env->GetObjectClass(sharedCallbackObj);
                if (LIKELY(clazz))
                {
                    isharedCallback_fields.onShared =
                            env->GetMethodID(clazz, "onShared", "(I)V");
                } else
                {
                    LOGW("failed to get object class");
                }
                env->ExceptionClear();
                if (!isharedCallback_fields.onShared)
                {
                    LOGE("Can't find ISharedCallback#onShared");
                    env->DeleteGlobalRef(sharedCallbackObj);
                    mSharedCallbackObj = sharedCallbackObj = NULL;
                }
            }
        }
    } else
    {
        if (LIKELY(mSharedCallbackObj))
        {
            env->DeleteGlobalRef(mSharedCallbackObj);
            mSharedCallbackObj = NULL;
        }
    }
    return mMemoryFileObj;
}

void Shared::closeShared(JNIEnv *env)
{
    if (LIKELY(mSharedCallbackObj))
    {
        env->DeleteGlobalRef(mSharedCallbackObj);
        mSharedCallbackObj = NULL;
    }
    if (LIKELY(mMemoryFileObj))
    {
        env->CallVoidMethod(mMemoryFileObj, memoryFile_fields.close);
        env->DeleteGlobalRef(mMemoryFileObj);
        mMemoryFileObj = NULL;
    }
    if (LIKELY(mMemoryFileArr))
    {
        env->DeleteGlobalRef(mMemoryFileArr);
        mMemoryFileArr = NULL;
    }
}

void Shared::onShared(JNIEnv *env, void *frame_data, int frame_data_size)
{
    if (LIKELY(mMemoryFileArr) && LIKELY(mMemoryFileObj))
    {
        // 0-15位都是头信息
        int length = env->GetArrayLength(mMemoryFileArr);
        int size = min(length - 16, frame_data_size);
        env->SetByteArrayRegion(mMemoryFileArr, 0, 4, (jbyte *) &frame_data_size);
        env->SetByteArrayRegion(mMemoryFileArr, 16, size, (jbyte *) frame_data);
        env->CallVoidMethod(mMemoryFileObj, memoryFile_fields.writeBytes, mMemoryFileArr,
                            0, 0, size + 16);
        if (LIKELY(mSharedCallbackObj))
        {
            env->CallVoidMethod(mSharedCallbackObj, isharedCallback_fields.onShared, size);
        }
    }
}
