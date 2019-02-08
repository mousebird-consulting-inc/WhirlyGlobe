/*
 *  WhirlyKitLog.h
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2019 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import <cstdio>
#import <stdlib.h>
#import <string>
#import <Foundation/Foundation.h>
#import "WhirlyKitLog.h"

/*  Note: Move these over to Android
#ifdef ANDROID
#include <android/log.h>

#define  LOG_TAG    "WhirlyKit"

#define WHIRLYKIT_LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,__VA_ARGS__)
#define WHIRLYKIT_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG,__VA_ARGS__)
#define WHIRLYKIT_LOGI(...) __android_log_print(ANDROID_LOG_INFO   , LOG_TAG,__VA_ARGS__)
#define WHIRLYKIT_LOGW(...) __android_log_print(ANDROID_LOG_WARN   , LOG_TAG,__VA_ARGS__)
#define WHIRLYKIT_LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , LOG_TAG,__VA_ARGS__)
#define WHIRLYKIT_LOGSIMPLE LOGSIMPLE

 */

void wkLogInternal(const char *formatStr,va_list args)
{
    // Scan once for the size and then form the string
    int len = std::vsnprintf(NULL,0,formatStr,args);
    
    char str[len+2];
    std::vsnprintf(str,len+1,formatStr,args);
    
    NSLog(@"%s",str);
}

void wkLog(const char *formatStr,...)
{
    va_list args;
    
    va_start(args, formatStr);
    wkLogInternal(formatStr, args);
    va_end(args);
}

static const char *levels[] = {"Verbose","Debug","Info","Warn","Error"};

void wkLogLevel(WKLogLevel level,const char *formatStr,...)
{
    va_list args;
    va_start(args, formatStr);

    std::string fullFormatStr = std::string(levels[level]) + ": " + formatStr;
    wkLogInternal(fullFormatStr.c_str(),args);
    
    va_end(args);
}
