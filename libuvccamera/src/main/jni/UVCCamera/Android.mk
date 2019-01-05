######################################################################
# Make shared library libUVCCamera.so
######################################################################
LOCAL_PATH	:= $(call my-dir)
include $(CLEAR_VARS)

######################################################################
# Make shared library libUVCCamera.so
######################################################################
CFLAGS := -Werror

LOCAL_C_INCLUDES := \
		$(LOCAL_PATH)/ \
		$(LOCAL_PATH)/../ \

LOCAL_CFLAGS := $(LOCAL_C_INCLUDES:%=-I%)
LOCAL_CFLAGS += -DANDROID_NDK
LOCAL_CFLAGS += -DLOG_NDEBUG
LOCAL_CFLAGS += -DACCESS_RAW_DESCRIPTORS
LOCAL_CFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -ldl
LOCAL_LDLIBS += -llog
LOCAL_LDLIBS += -landroid

LOCAL_SHARED_LIBRARIES += jpeg-turbo-1.5.3 usb uvc
# LOCAL_STATIC_LIBRARIES += 

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
		_onload.cpp \
		utilbase.cpp \
		objcet_frame_stream.cpp \
		Mutex.cpp \
		TurboJpegUtils.cpp \
		ILooper.cpp \
		FramePool.cpp \
		Shared.cpp \
		AbstractCapture.cpp \
		AbstractPreview.cpp \
		UVCCapture.cpp \
		V4L2Capture.cpp \
		MjpegPreview.cpp \
		UVCCamera.cpp \
		com_cit_uvccamera_UVCCamera.cpp


LOCAL_MODULE    := UVCCamera
include $(BUILD_SHARED_LIBRARY)
