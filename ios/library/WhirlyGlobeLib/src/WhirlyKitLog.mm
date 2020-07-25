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

void wkLog(const char *formatStr,...)
{
    va_list args;
    
    va_start(args, formatStr);
    int len = std::vsnprintf(NULL,0,formatStr,args);
    if (len < 0) {
        NSLog(@"wkLogLevel: Malformed format string");
        return;
    }

    va_end(args);
    va_start(args, formatStr);
    
    char str[len+2];
    std::vsnprintf(str,len+1,formatStr,args);
    
    NSLog(@"%s",str);
    
    va_end(args);
}

static const char *levels[] = {"Verbose","Debug","Info","Warn","Error"};

void wkLogLevel(WKLogLevel level,const char *formatStr,...)
{
    va_list args;
    va_start(args, formatStr);

    std::string fullFormatStr = std::string(levels[level]) + ": " + formatStr;

    // Scan once for the size and then form the string
    int len = std::vsnprintf(NULL,0,formatStr,args);
    if (len < 0) {
        NSLog(@"wkLogLevel: Malformed format string");
        return;
    }

    va_end(args);
    va_start(args, formatStr);
    
    char str[len+2];
    std::vsnprintf(str,len+1,formatStr,args);
    
    NSLog(@"%s",str);
    
    va_end(args);
}
