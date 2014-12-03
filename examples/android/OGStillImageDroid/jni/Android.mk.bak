# Must define the LOCAL_PATH and return the current dir
LOCAL_PATH := $(call my-dir)
# Cleans various variables... making a clean build
include $(CLEAR_VARS)
# Identify the module/library's name
LOCAL_MODULE := jni_img_proc
# Specify the source files
LOCAL_SRC_FILES := jni_img_proc.cpp
# Load local libraries
LOCAL_LDLIBS += -llog# Build the shared library defined above
include $(BUILD_SHARED_LIBRARY)