APP_CPPFLAGS += -Wno-deprecated 
APP_CPPFLAGS += -std=c++11
APP_CPPFLAGS += -frtti
APP_CPPFLAGS += -fexceptions
APP_CFLAGS += -D__USE_SDL_GLES__
APP_STL := gnustl_static
# APP_ABI := x86
APP_ABI := x86 armeabi armeabi-v7a mips
# APP_CPPFLAGS += -g -O0
# NDK_DEBUG := 1
APP_PLATFORM := android-18
