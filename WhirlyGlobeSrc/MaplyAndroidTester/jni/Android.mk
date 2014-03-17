LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := MaplyTester
LOCAL_SRC_FILES := MaplyTester.cpp

APP_ABI := x86
APP_CPPFLAGS += -g
NDK_DEBUG=1

include $(BUILD_SHARED_LIBRARY)
