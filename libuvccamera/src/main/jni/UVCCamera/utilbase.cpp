#include "_utilbase.h"

static JavaVM *savedVm;

void setVM(JavaVM *vm) {
	savedVm = vm;
}

JavaVM *getVM() {
	return savedVm;
}

JNIEnv *getEnv() {
    JNIEnv *env = NULL;
    if (savedVm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
    	env = NULL;
    }
    return env;
}

jobject getGlobalContext(JNIEnv *env) 
{
	jobject context = NULL;

	jclass activityThread = env->FindClass("android/app/ActivityThread");

	jmethodID currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");

	jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);

	if (at)
	{
		jmethodID getApplication = env->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");

		context = env->CallObjectMethod(at, getApplication);

		env->DeleteLocalRef(at);
	}

	return context;
}
