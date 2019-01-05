#ifndef ONLOAD_H
#define ONLOAD_H

#pragma interface

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

jint JNI_OnLoad(JavaVM *vm, void *reserved);

#ifdef __cplusplus
}
#endif

#endif //ONLOAD_H
