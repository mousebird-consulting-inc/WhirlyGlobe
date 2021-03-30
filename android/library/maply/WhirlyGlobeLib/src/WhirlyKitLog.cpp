/*
 *  WhirlyKitLog.cpp
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
#import "WhirlyKitLog.h"
#import <android/log.h>

void wkLog(const char *formatStr,...)
{
    va_list args;
    
    va_start(args, formatStr);
    __android_log_vprint(ANDROID_LOG_INFO, "Maply", formatStr, args);
    va_end(args);
}

//static const char *levels[] = {"Verbose","Debug","Info","Warn","Error"};

void wkLogLevel_(WKLogLevel level,const char *formatStr,...)
{
    va_list args;
    va_start(args, formatStr);
    
    int androidLevel = ANDROID_LOG_VERBOSE;
    switch (level)
    {
        case Verbose:
            androidLevel = ANDROID_LOG_VERBOSE;
            break;
        case Debug:
            androidLevel = ANDROID_LOG_DEBUG;
            break;
        case Info:
            androidLevel = ANDROID_LOG_INFO;
            break;
        case Warn:
            androidLevel = ANDROID_LOG_WARN;
            break;
        case Error:
            androidLevel = ANDROID_LOG_ERROR;
            break;
        default:
            break;
    }

    __android_log_vprint(androidLevel, "Maply", formatStr, args);
    
    va_end(args);
}
