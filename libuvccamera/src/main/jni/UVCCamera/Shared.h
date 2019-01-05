#ifndef SHARED_H
#define SHARED_H

#include "_utilbase.h"

typedef struct
{
    jmethodID close;
    jmethodID readBytes;
    jmethodID writeBytes;
} Fields_memoryFile;

typedef struct
{
    jmethodID onShared;
} Fields_isharedCallback;

class Shared
{
public:
    Shared();
    virtual ~Shared();

private:
    jbyteArray mMemoryFileArr;
    jobject mMemoryFileObj;
    Fields_memoryFile memoryFile_fields;
    jobject mSharedCallbackObj;
    Fields_isharedCallback isharedCallback_fields;

public:
    jobject openShared(JNIEnv *env, int length, jobject sharedCallback);
    void closeShared(JNIEnv *env);
    void onShared(JNIEnv *env, void *frame_data, int frame_data_size);
};


#endif //SHARED_H
