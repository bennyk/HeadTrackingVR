
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := lynda-demo
LOCAL_CFLAGS    := -Wall -DGL_GLEXT_PROTOTYPES -fexceptions

LOCAL_LDLIBS    := -llog -landroid
LOCAL_LDLIBS += -lEGL -lGLESv2
#LOCAL_LDLIBS += -lEGL -lGLESv1_CM
LOCAL_SRC_FILES := jniapi.cpp 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include $(LOCAL_PATH)/glm $(LOCAL_PATH)/examples

include $(BUILD_SHARED_LIBRARY)

