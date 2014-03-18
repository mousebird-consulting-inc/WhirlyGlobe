LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := MaplyTester
LOCAL_SRC_FILES := MaplyTester.cpp

include $(BUILD_SHARED_LIBRARY)
