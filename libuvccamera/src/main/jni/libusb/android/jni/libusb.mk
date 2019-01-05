######################################################################
# libusb.a
######################################################################
LOCAL_PATH			:= $(call my-dir)/../..
include $(CLEAR_VARS)

# changed linux_usbfs.c => android_usbfs.c
# changed linux_netlink.c => android_netlink.c
# these sources are also modified.
LOCAL_SRC_FILES := \
	libusb/core.c \
	libusb/descriptor.c \
	libusb/hotplug.c \
	libusb/io.c \
	libusb/sync.c \
	libusb/strerror.c \
	libusb/os/android_usbfs.c \
	libusb/os/poll_posix.c \
	libusb/os/threads_posix.c \
	libusb/os/android_netlink.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/ \
	$(LOCAL_PATH)/libusb \
	$(LOCAL_PATH)/libusb/os \
	$(LOCAL_PATH)/../ \

LOCAL_EXPORT_C_INCLUDES := \
	$(LOCAL_PATH)/ \
	$(LOCAL_PATH)/libusb

# add some flags
LOCAL_CFLAGS := $(LOCAL_C_INCLUDES:%=-I%)
LOCAL_CFLAGS += -DANDROID_NDK
LOCAL_CFLAGS += -DLOG_NDEBUG
LOCAL_CFLAGS += -DACCESS_RAW_DESCRIPTORS
LOCAL_CFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays
LOCAL_EXPORT_LDLIBS += -llog
LOCAL_ARM_MODE := arm

LOCAL_MODULE := usb_static
include $(BUILD_STATIC_LIBRARY)

######################################################################
# libusb.so
######################################################################
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_EXPORT_LDLIBS += -llog

LOCAL_WHOLE_STATIC_LIBRARIES = usb_static

LOCAL_MODULE := usb
include $(BUILD_SHARED_LIBRARY)
